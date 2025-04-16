void
MavlinkReceiver::acknowledge(uint8_t sysid, uint8_t compid, uint16_t command, uint8_t result, uint8_t progress)
{
	vehicle_command_ack_s command_ack{};

	command_ack.timestamp = hrt_absolute_time();
	command_ack.command = command;
	command_ack.result = result;
	command_ack.target_system = sysid;
	command_ack.target_component = compid;
	command_ack.result_param1 = progress;

	_cmd_ack_pub.publish(command_ack);
}