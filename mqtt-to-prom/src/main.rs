#[macro_use]
extern crate prometheus;
#[macro_use]
extern crate lazy_static;
#[macro_use]
extern crate log;
extern crate pretty_env_logger;

use prometheus::{Encoder, GaugeVec, TextEncoder};
use rumqttc::{Client, Event, Incoming, MqttOptions, QoS};
use std::env;
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
    pretty_env_logger::init();
    info!("Starting mqtt thread...");

    let mqtt_host = env::var("MQTT_HOST").unwrap_or(String::from("localhost"));
    let bind_address = env::var("TCP_BIND_ADDR").unwrap_or(String::from("localhost:19090"));

    info!("mqtt_host {}", mqtt_host);
    info!("bind_address {}", bind_address);

    thread::spawn(|| {
        let mut mqttoptions = MqttOptions::new("esp32-server", mqtt_host, 1883);
        mqttoptions.set_keep_alive(5);

        let (mut client, mut connection) = Client::new(mqttoptions, 10);
        client.subscribe("sensors/thp/+", QoS::AtMostOnce).unwrap();

        // Iterate to poll the eventloop for connection progress
        for (_, notification) in connection.iter().enumerate() {
            debug!("Received {:?}", notification);
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

    let listener = TcpListener::bind(bind_address).unwrap();

    info!("Starting server...");

    for stream in listener.incoming() {
        debug!("Request received");
        let mut stream: TcpStream = stream.unwrap();

        let mut req = [0; 1024];

        stream.read(&mut req).unwrap();

        debug!("req: {}", String::from_utf8_lossy(&req[..]));

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
