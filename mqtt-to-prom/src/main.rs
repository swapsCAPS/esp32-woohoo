#[macro_use]
extern crate prometheus;
#[macro_use]
extern crate lazy_static;

use prometheus::{Encoder, GaugeVec, TextEncoder};
use rumqttc::{Client, Event, Incoming, MqttOptions, QoS};
use std::io::prelude::*;
use std::net::{TcpListener, TcpStream};
use std::str;
use std::thread;

lazy_static! {
    static ref TEMPERATURE_GAUGE: GaugeVec =
        register_gauge_vec!("esp32_temperature", "Temperature", &["mac"]).unwrap();
    static ref HUMIDITY_GAUGE: GaugeVec =
        register_gauge_vec!("esp32_humidity", "Humidity", &["mac"]).unwrap();
    static ref PRESSURE_GAUGE: GaugeVec =
        register_gauge_vec!("esp32_pressure", "Pressure", &["mac"]).unwrap();
}

fn main() {
    thread::spawn(|| {
        let mut mqttoptions = MqttOptions::new("esp32-server", "192.168.178.20", 1883);
        mqttoptions.set_keep_alive(5);

        let (mut client, mut connection) = Client::new(mqttoptions, 10);
        client.subscribe("sensors/thp/+", QoS::AtMostOnce).unwrap();

        // Iterate to poll the eventloop for connection progress
        for (_, notification) in connection.iter().enumerate() {
            if let Ok(Event::Incoming(Incoming::Publish(p))) = notification {
                let mac = p.topic.split("/").last().unwrap();
                let payload = str::from_utf8(&p.payload).unwrap();

                let split: Vec<&str> = payload.split(",").collect();

                if let [temperature, humidity, pressure] = &split[..] {
                    TEMPERATURE_GAUGE
                        .with_label_values(&[mac])
                        .set(temperature.parse::<f64>().unwrap());
                    HUMIDITY_GAUGE
                        .with_label_values(&[mac])
                        .set(humidity.parse::<f64>().unwrap());
                    PRESSURE_GAUGE
                        .with_label_values(&[mac])
                        .set(pressure.parse::<f64>().unwrap());
                }
            }
        }
    });

    let listener = TcpListener::bind("127.0.0.1:19090").unwrap();

    for stream in listener.incoming() {
        let mut stream: TcpStream = stream.unwrap();

        let mut buffer = vec![];
        let encoder = TextEncoder::new();
        let metric_families = prometheus::gather();
        encoder.encode(&metric_families, &mut buffer).unwrap();

        let res = format!(
            "HTTP/1.1 200 OK\r\nContent-Length: {}\r\n\r\n{}",
            buffer.len(),
            str::from_utf8(&buffer).unwrap()
        );

        stream.write(res.as_bytes()).unwrap();
        stream.flush().unwrap();
    }
}
