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
    static ref BATTERY_LEVEL: GaugeVec =
        register_gauge_vec!("esp32_bat_lvl", "Battery level", &["mac"]).unwrap();
    static ref EPOCH: GaugeVec = register_gauge_vec!("esp32_epoch", "Epoch", &["mac"]).unwrap();
    static ref MEASUREMENT_SUCCESS: GaugeVec = register_gauge_vec!(
        "esp32_measurement_success",
        "Times taking a measurement succeeded",
        &["mac"]
    )
    .unwrap();
    static ref MEASUREMENT_FAILURE: GaugeVec = register_gauge_vec!(
        "esp32_measurement_failure",
        "Times measurement failed",
        &["mac"]
    )
    .unwrap();
}

fn main() {
    pretty_env_logger::init();
    info!("Starting mqtt thread...");

    let mqtt_host = env::var("MQTT_HOST").unwrap_or(String::from("localhost"));
    let bind_address = env::var("TCP_BIND_ADDR").unwrap_or(String::from("localhost:19090"));

    info!("mqtt_host {}", mqtt_host);
    info!("bind_address {}", bind_address);

    // --- MQTT Thread ---
    thread::spawn(move || {
        let mut mqttoptions = MqttOptions::new("esp32-server", mqtt_host, 1883);
        mqttoptions.set_keep_alive(5);

        let (mut client, mut connection) = Client::new(mqttoptions, 10);

        // Wrap subscription in case of early errors
        if let Err(e) = client.subscribe("sensors/thp/+", QoS::AtMostOnce) {
            error!("Failed to subscribe: {:?}", e);
        }

        // Process MQTT notifications
        for (i, notification) in connection.iter().enumerate() {
            // Every 1000 events, log a heartbeat to see if this loop is flooding
            if i % 1000 == 0 {
                debug!("MQTT Loop Heartbeat: processed {} events", i);
            }

            match notification {
                Ok(Event::Incoming(Incoming::Publish(p))) => {
                    let mac = match p.topic.split("/").last() {
                        Some(m) => m,
                        None => {
                            warn!("Failed to extract mac from topic: {}", p.topic);
                            continue;
                        }
                    };

                    let payload = match str::from_utf8(&p.payload) {
                        Ok(v) => v,
                        Err(_) => {
                            warn!("Invalid UTF-8 payload");
                            continue;
                        }
                    };

                    let split: Vec<&str> = payload.split(",").collect();

                    if let [temp, h, p, b, epoch, success, failure] = &split[..] {
                        if let Ok(result) = temp.trim().parse::<f64>() {
                            TEMPERATURE_GAUGE
                                .with_label_values(&[mac])
                                .set((result * 4.0).round() / 4.0);
                        }
                        if let Ok(result) = h.trim().parse::<f64>() {
                            HUMIDITY_GAUGE
                                .with_label_values(&[mac])
                                .set((result * 4.0).round() / 4.0);
                        }
                        if let Ok(result) = p.trim().parse::<f64>() {
                            PRESSURE_GAUGE
                                .with_label_values(&[mac])
                                .set((result * 4.0).round() / 4.0);
                        }
                        if let Ok(result) = b.trim().parse::<f64>() {
                            BATTERY_LEVEL.with_label_values(&[mac]).set(result);
                        }
                        if let Ok(result) = epoch.trim().parse::<f64>() {
                            EPOCH.with_label_values(&[mac]).set(result);
                        }
                        if let Ok(result) = success.trim().parse::<f64>() {
                            MEASUREMENT_SUCCESS.with_label_values(&[mac]).set(result);
                        }
                        if let Ok(result) = failure.trim().parse::<f64>() {
                            MEASUREMENT_FAILURE.with_label_values(&[mac]).set(result);
                        }
                    } else {
                        warn!("Could not split payload from {}", payload);
                    }
                }
                Ok(_other) => {
                    // Ignore non-publish events (pings, acks, etc) to prevent log flooding
                }
                Err(e) => {
                    error!("MQTT connection error: {:?}", e);
                    // Crucial: Sleep on error to prevent spinning during broker outage
                    thread::sleep(std::time::Duration::from_secs(2));
                }
            }
        }
    });

    // --- TCP Server (Prometheus Scrape Endpoint) ---
    let listener = TcpListener::bind(bind_address).unwrap();
    info!("Starting server...");

    for stream in listener.incoming() {
        let mut stream = match stream {
            Ok(s) => s,
            Err(e) => {
                error!("TCP incoming stream connection error: {:?}", e);
                thread::sleep(std::time::Duration::from_millis(50)); // Prevent hot loop if OS runs out of FDs
                continue;
            }
        };

        debug!("Request received");
        let mut req = [0; 1024];

        // Read from stream defensively
        match stream.read(&mut req) {
            Ok(0) => {
                // Client connected and immediately closed (EOF)
                debug!("Client closed connection immediately (0 bytes read)");
                continue;
            }
            Ok(n) => {
                debug!("Read {} bytes", n);
            }
            Err(e) => {
                warn!("Failed to read from TCP stream: {:?}", e);
                continue;
            }
        }

        let mut buffer = vec![];
        let encoder = TextEncoder::new();
        let metric_families = prometheus::gather();
        encoder.encode(&metric_families, &mut buffer).unwrap();

        let res = format!(
            "HTTP/1.1 200 OK\r\nContent-Length: {}\r\nConnection: close\r\n\r\n{}",
            buffer.len(),
            str::from_utf8(&buffer).unwrap()
        );

        if let Err(e) = stream.write_all(res.as_bytes()) {
            warn!("Failed to write TCP response: {:?}", e);
        }
        let _ = stream.flush();
    }
}
