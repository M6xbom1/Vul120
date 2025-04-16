static void
input_l2cap_frame_flow_channel(l2cap_channel_t *channel, uint8_t *data, uint16_t data_len)
{
  uint16_t frame_len;
  uint16_t payload_len;

  if(data_len < 4) {
    LOG_WARN("l2cap_frame: illegal L2CAP frame data_len: %d\n", data_len);
    /* a L2CAP frame has a minimum length of 4 */
    return;
  }

  if(channel->rx_buffer.sdu_length == 0) {
    /* handle first fragment */
    memcpy(&frame_len, &data[0], 2);
    payload_len = frame_len - 2;

    if(payload_len > BLE_L2CAP_NODE_MTU) {
    	LOG_WARN("l2cap_frame: illegal L2CAP frame payload_len: %d\n", payload_len);
    	/* the payload length may not be larger than the destination buffer */
    	return;
    }

    memcpy(&channel->rx_buffer.sdu_length, &data[4], 2);

    memcpy(channel->rx_buffer.sdu, &data[6], payload_len);
    channel->rx_buffer.current_index = payload_len;
  } else {
    /* subsequent fragment */
    memcpy(&frame_len, &data[0], 2);
    payload_len = frame_len;
    
    if(channel->rx_buffer.current_index + payload_len > BLE_L2CAP_NODE_MTU) {
    	LOG_WARN("l2cap_frame: illegal L2CAP frame payload_len: %d\n", payload_len);
    	/* the current index plus the payload length may not be larger than 
	 * the destination buffer */
    	return;
    }

    memcpy(&channel->rx_buffer.sdu[channel->rx_buffer.current_index], &data[4], payload_len);
    channel->rx_buffer.current_index += payload_len;
  }

  if((channel->rx_buffer.sdu_length > 0) &&
     (channel->rx_buffer.sdu_length == channel->rx_buffer.current_index)) {
    /* do not use packetbuf_copyfrom here because the packetbuf_attr
     * must not be cleared */
    memcpy(packetbuf_dataptr(), channel->rx_buffer.sdu, channel->rx_buffer.sdu_length);
    packetbuf_set_datalen(channel->rx_buffer.sdu_length);
    NETSTACK_NETWORK.input();

    /* reset counters */
    channel->rx_buffer.sdu_length = 0;
    channel->rx_buffer.current_index = 0;
  }
}