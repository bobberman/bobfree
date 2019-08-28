#include "c_client_state_.h"
#include "../utils/c_hook.h"
#include "../utils/c_config.h"
#include "c_prediction_.h"
#include "../sdk/c_cvar.h"
#include "../sdk/c_game_rules.h"
#include "c_events.h"

void c_client_state_::hook()
{
	static c_hook<c_client_state> hook(reinterpret_cast<c_client_state*>(uint32_t(client_state) + 8));
	_packet_start = hook.apply<packet_start_t>(5, packet_start);
}

void __fastcall c_client_state_::packet_start(c_client_state* state, uint32_t, int incoming_sequence, int outgoing_acknowledged)
{
	return _packet_start(state, incoming_sequence, outgoing_acknowledged);
}

c_net_graph_system* c_net_graph_system::get()
{
	static const auto netgraph = **reinterpret_cast<c_net_graph_system***>(sig("client_panorama.dll", "89 1D ? ? ? ? 8B C3") + 0x2);
	return netgraph;
}