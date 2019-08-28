#include "c_net_channel_.h"
#include "../hacks/c_fake_ping.h"

void c_net_channel_::hook()
{
	//static uint32_t dummy[1] = { reinterpret_cast<uint32_t>(c_net_channel::get_vtable()) };
	//hk = std::make_unique<c_hook<uint32_t>>(dummy);

	//_process_packet = hk->apply<process_packet_t>(39, process_packet);
	//_send_netmsg = hk->apply<send_netmsg_t>(40, send_netmsg);
	//_send_datagram = hk->apply<send_datagram_t>(46, send_datagram);
}

void c_net_channel_::apply_to_net_chan(c_net_channel* channel)
{
	/*bandaid fix, started throwing vgui2.dll nullptrs with patchpointer - ML*/

	hk = std::make_unique<c_hook<uint32_t>>((uint32_t*)channel);
	//hk = std::make_unique<c_hook<uint32_t>>((uint32_t*)channel);
	_send_datagram = hk->apply<send_datagram_t>(46, send_datagram);
	//hk->patch_pointer(reinterpret_cast<uintptr_t*>(channel));
}

void __fastcall c_net_channel_::process_packet(c_net_channel* channel, uint32_t, void* packet, bool header)
{
	_process_packet(channel, packet, header);

	const auto local = c_cs_player::get_local_player();

	if (engine_client()->is_ingame() && local && local->is_alive())
		fake_ping->flip_state(channel);
}

bool c_net_channel_::send_netmsg(c_net_channel* channel, uint32_t, c_net_msg* msg, const bool reliable, const bool voice)
{
	return _send_netmsg(channel, msg, reliable, msg->get_group() == 9 ? true : voice);
}
void c_net_channel_::UpdateIncomingSequences(c_net_channel* channel) {
	static int lastseq = -1;
	//if (lastseq == channel->in_sequence_nr)return;

	c_net_channel_::sequences.push_front(CIncomingSequence(channel->in_reliable_state, 0, 0, channel->in_sequence_nr, global_vars_base->curtime));
	if (c_net_channel_::sequences.size() > 2048)
		c_net_channel_::sequences.pop_back();
}
int c_net_channel_::send_datagram(c_net_channel* channel, uint32_t, void* buffer){
	if (!engine_client()->is_ingame()|| !c_cs_player::get_local_player() || buffer)
			return _send_datagram(channel, buffer);

	UpdateIncomingSequences(channel);
	const auto backup_in_seq = channel->in_sequence_nr;
	const auto backup_reliable = channel->in_reliable_state;//new

	fake_ping->set_suitable_in_sequence(channel);

	const auto original = _send_datagram(channel, buffer);

	channel->in_sequence_nr = backup_in_seq;
	channel->in_reliable_state = backup_reliable;//new
	return original;
}
