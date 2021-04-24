run-pi:
	RUST_LOG=info TCP_BIND_ADDR=192.168.178.20:19090 ./bin/mqtt-to-prom_aarch64-unknown-linux-gnu

build:
	TARGET_CC=aarch64-none-linux-gnu-gcc cargo build --target=aarch64-unknown-linux-gnu --release
