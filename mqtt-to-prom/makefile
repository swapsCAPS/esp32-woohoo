run-pi:
	RUST_LOG=info TCP_BIND_ADDR=192.168.178.20:19090 ./bin/mqtt-to-prom_aarch64-unknown-linux-gnu

build-pi:
	cross build --release --target aarch64-unknown-linux-gnu
	cp target/aarch64-unknown-linux-gnu/release/mqtt-to-prom bin/mqtt-to-prom_aarch64-unknown-linux-gnu

install-pi:
	sudo systemctl stop mqtt-to-prom
	sudo cp bin/mqtt-to-prom_aarch64-unknown-linux-gnu /usr/bin/mqtt-to-prom
	sudo cp meta/mqtt-to-prom.service /etc/systemd/system/.
	sudo systemctl enable mqtt-to-prom
	sudo systemctl start mqtt-to-prom
