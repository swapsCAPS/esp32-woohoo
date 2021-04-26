run-pi:
	RUST_LOG=info TCP_BIND_ADDR=192.168.178.20:19090 ./bin/mqtt-to-prom_aarch64-unknown-linux-gnu

build-pi:
	TARGET_CC=aarch64-none-linux-gnu-gcc cargo build --target=aarch64-unknown-linux-gnu --release
	cp target/aarch64-unknown-linux-gnu/release/mqtt-to-prom bin/mqtt-to-prom_aarch64-unknown-linux-gnu

install-pi:
	cp bin/mqtt-to-prom_aarch64-unknown-linux-gnu /usr/bin/mqtt-to-prom
	cp meta/mqtt-to-prom.service /etc/systemd/system/.
	systemctl enable mqtt-to-prom
	systemctl start mqtt-to-prom
