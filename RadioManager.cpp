#include "RadioManager.h"
#include "security/string_obfuscation.h"

RadioManager g_RadioManager;

RadioManager::RadioManager()
{
	// notes to anybody adding a radio: 
	//	order of stations doesn't matter now
	//	always remember to use _() on all your strings
	this->stations.push_back({ _("Smileycast"),		_("https://eu2.radiocastvps.com/radio/8000/radio.mp3?1562422553") });
	this->stations.push_back({ _("Anime"),			_("http://144.217.203.184:8972/listen.pls?sid=1&t=.m3u") });
	this->stations.push_back({ _("Techno"),			_("http://listen.technobase.fm/dsl.pls") });
	this->stations.push_back({ _("Big time"),		_("http://streams.bigfm.de/bigfm-deutschrap-128-aac") });
	this->stations.push_back({ _("Chill"),			_("http://mp3.stream.tb-group.fm/tt.mp3") });
	this->stations.push_back({ _("Club"),			_("http://mp3.stream.tb-group.fm/clt.mp3") });
	this->stations.push_back({ _("House"),			_("http://mp3.stream.tb-group.fm/ht.mp3") });
	this->stations.push_back({ _("Radio"),			_("http://www.iloveradio.de/iloveradio.m3u") });
	this->stations.push_back({ _("Rock"),			_("http://www.rockantenne.de/webradio/channels/alternative.m3u") });
	this->stations.push_back({ _("Hardstyle"),		_("http://listen.hardbase.fm/aac-hd.pls") });
	this->stations.push_back({ _("Rap / Hip Hop"),	_("http://192.211.51.158:8010/listen.pls?sid=1") });
	this->stations.push_back({ _("Deutschrap"),		_("http://deutschrap-high.rautemusik.fm/listen.pls") });
	this->stations.push_back({ _("J-Pop"),			_("http://66.70.187.44:9029/listen.pls?sid=1") });
	this->stations.push_back({ _("Gabber"),			_("http://centova.pure-isp.eu:8100/low_autodj.m3u") });
	this->stations.push_back({ _("Hardcore #1"),	_("http://81.18.165.235:80/listen.pls") });
	this->stations.push_back({ _("Hardcore #2"),	_("http://83.240.65.107:8000/hardcore.m3u") });
	this->stations.push_back({ _("Hardcore #3"),	_("http://93.158.201.101:8100/ultra_autodj") });
	this->stations.push_back({ _("Speedcore"),		_("http://listen.coretime.fm/aac-hd.pls") });
	this->stations.push_back({ _("Trap"),			_("http://radio.trap.fm/listen192.m3u") });
	this->stations.push_back({ _("Dubstep"),		_("https://www.internet-radio.com/servers/tools/playlistgenerator/?u=http://www.partyviberadio.com:8040/listen.pls?sid=1&t=.pls") });
	this->stations.push_back({ _("LoFi Chill"),		_("https://www.chillsky.com/listen/") });
}


RadioManager::~RadioManager()
{

}