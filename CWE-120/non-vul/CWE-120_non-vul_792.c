void
MavlinkReceiver::handle_message_hil_sensor(mavlink_message_t *msg)
{
	mavlink_hil_sensor_t hil_sensor;
	mavlink_msg_hil_sensor_decode(msg, &hil_sensor);

	const uint64_t timestamp = hrt_absolute_time();

	// temperature only updated with baro
	float temperature = NAN;

	if ((hil_sensor.fields_updated & SensorSource::BARO) == SensorSource::BARO) {
		temperature = hil_sensor.temperature;
	}

	// gyro
	if ((hil_sensor.fields_updated & SensorSource::GYRO) == SensorSource::GYRO) {
		if (_px4_gyro == nullptr) {
			// 1310988: DRV_IMU_DEVTYPE_SIM, BUS: 1, ADDR: 1, TYPE: SIMULATION
			_px4_gyro = new PX4Gyroscope(1310988);
		}

		if (_px4_gyro != nullptr) {
			if (PX4_ISFINITE(temperature)) {
				_px4_gyro->set_temperature(temperature);
			}

			_px4_gyro->update(timestamp, hil_sensor.xgyro, hil_sensor.ygyro, hil_sensor.zgyro);
		}
	}

	// accelerometer
	if ((hil_sensor.fields_updated & SensorSource::ACCEL) == SensorSource::ACCEL) {
		if (_px4_accel == nullptr) {
			// 1310988: DRV_IMU_DEVTYPE_SIM, BUS: 1, ADDR: 1, TYPE: SIMULATION
			_px4_accel = new PX4Accelerometer(1310988);
		}

		if (_px4_accel != nullptr) {
			if (PX4_ISFINITE(temperature)) {
				_px4_accel->set_temperature(temperature);
			}

			_px4_accel->update(timestamp, hil_sensor.xacc, hil_sensor.yacc, hil_sensor.zacc);
		}
	}

	// magnetometer
	if ((hil_sensor.fields_updated & SensorSource::MAG) == SensorSource::MAG) {
		if (_px4_mag == nullptr) {
			// 197388: DRV_MAG_DEVTYPE_MAGSIM, BUS: 3, ADDR: 1, TYPE: SIMULATION
			_px4_mag = new PX4Magnetometer(197388);
		}

		if (_px4_mag != nullptr) {
			if (PX4_ISFINITE(temperature)) {
				_px4_mag->set_temperature(temperature);
			}

			_px4_mag->update(timestamp, hil_sensor.xmag, hil_sensor.ymag, hil_sensor.zmag);
		}
	}

	// baro
	if ((hil_sensor.fields_updated & SensorSource::BARO) == SensorSource::BARO) {
		if (_px4_baro == nullptr) {
			// 6620172: DRV_BARO_DEVTYPE_BAROSIM, BUS: 1, ADDR: 4, TYPE: SIMULATION
			_px4_baro = new PX4Barometer(6620172);
		}

		if (_px4_baro != nullptr) {
			_px4_baro->set_temperature(hil_sensor.temperature);
			_px4_baro->update(timestamp, hil_sensor.abs_pressure);
		}
	}

	// differential pressure
	if ((hil_sensor.fields_updated & SensorSource::DIFF_PRESS) == SensorSource::DIFF_PRESS) {
		differential_pressure_s report{};
		report.timestamp = timestamp;
		report.temperature = hil_sensor.temperature;
		report.differential_pressure_filtered_pa = hil_sensor.diff_pressure * 100.0f; // convert from millibar to bar;
		report.differential_pressure_raw_pa = hil_sensor.diff_pressure * 100.0f; // convert from millibar to bar;

		_differential_pressure_pub.publish(report);
	}

	// battery status
	{
		battery_status_s hil_battery_status{};

		hil_battery_status.timestamp = timestamp;
		hil_battery_status.voltage_v = 16.0f;
		hil_battery_status.voltage_filtered_v = 16.0f;
		hil_battery_status.current_a = 10.0f;
		hil_battery_status.discharged_mah = -1.0f;
		hil_battery_status.connected = true;
		hil_battery_status.remaining = 0.70;

		_battery_pub.publish(hil_battery_status);
	}
}