use rumqttc::{Client, Event, Incoming, MqttOptions, QoS};
use std::thread;
use std::time::Duration;

fn main() {
    let mut mqttoptions = MqttOptions::new("esp32-server", "192.168.178.20", 1883);
    mqttoptions.set_keep_alive(5);

    let (mut client, mut connection) = Client::new(mqttoptions, 10);
    client.subscribe("sensors/thp/+", QoS::AtMostOnce).unwrap();

    // TODO Add webserver

    // Iterate to poll the eventloop for connection progress
    for (i, notification) in connection.iter().enumerate() {
        if let Ok(Event::Incoming(Incoming::Publish(p))) = notification {
            println!("topic: {}, payload: {:?}", p.topic, p.payload);
            // TODO update state
        }
    }
}
