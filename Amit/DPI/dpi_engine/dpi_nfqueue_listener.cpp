#include <iostream>
#include <cstring>
#include <cstdint>
#include <tuple>
#include <csignal>
#include <map>
#include <memory>
#include <vector>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <bpf/bpf.h>

#define XDP_TRIE_MAP_PATH "/sys/fs/bpf/flow_trie_map"
#define XDP_HASH_MAP_PATH "/sys/fs/bpf/dpi_map"


typedef  unsigned long long uint64;

struct FlowKey {
    uint32_t ip1;
    uint32_t ip2;
    uint16_t port1;
    uint16_t port2;
    uint8_t  protocol;
    bool operator<(const FlowKey& other) const {
        return std::tie(ip1, ip2, port1, port2, protocol) <
               std::tie(other.ip1, other.ip2,
                        other.port1, other.port2,
                        other.protocol);
    }
};

class Flow {
	public:
		explicit Flow(const FlowKey& key);
		virtual ~Flow() = default;

		// Main DPI entry point
		virtual void process_packet(const uint8_t* data,
					    size_t len,
					    uint64_t timestamp_ns) = 0;

		const FlowKey& key() const { return key_; }
		uint64_t last_seen() const { return last_seen_ns_; }

	protected:
		FlowKey  key_;
		uint64_t last_seen_ns_;

		// DPI common state
		bool protocol_identified_;
};

class FlowManager {
public:
	explicit FlowManager();
	static FlowManager& instance() {
        	static FlowManager instance;
        	return instance;
    	}
	Flow* get_create_flow(const FlowKey& key);
	void expire(uint64_t now_ns);
	int update_xdp_map(unsigned int dest_ip);

private:
	std::map<FlowKey, std::unique_ptr<Flow>> flows_;
	int xdp_flow_map_fd;
	int xdp_dpi_map_fd;
};

class Packet {
	private:
		struct nfq_q_handle *qh_;
		unsigned int pkt_len;
		unsigned char *data;
		unsigned int payload_len;
		unsigned char *payload;
		unsigned char ipv4;
		unsigned char ipv6;
		unsigned int pkt_id;
		unsigned int s_ip;
		unsigned int d_ip;
		char is_tcp_;
		char is_udp_;
	public:
		explicit Packet(struct nfq_q_handle *qh, struct nfq_data *nf_data, FlowManager *flow_m);
		virtual ~Packet() = default;
		//virtual int parse(struct nfq_q_handle *qh, struct nfq_data *nf_d) = 0;
		virtual int parse() = 0;
		virtual FlowKey make_flow_key() const = 0;
		virtual char match_sni(char *stream) = 0;
		int verdict(int verdict);
		void *packet_start();
		void *l4_header_start();
		int ip_length();
		int ip_header_length();
		int is_valid();
		char is_tcp();
		char is_udp();
		unsigned int source_ip() const {
			return s_ip;
		}
		unsigned int dest_ip() const {
			return d_ip;
		}
};

Packet::Packet(struct nfq_q_handle *qh, struct nfq_data *nf_data, FlowManager *flow_m) {
	struct nfqnl_msg_packet_hdr *ph;
	struct iphdr *ip_hdr;
	payload_len = nfq_get_payload(nf_data, &payload);

	ip_hdr = (struct iphdr *)payload;
	ph = nfq_get_msg_packet_hdr(nf_data);
	if (ph)
		pkt_id = ntohl(ph->packet_id);

	s_ip = (unsigned int)(ip_hdr->addrs.saddr);
	d_ip = (unsigned int)(ip_hdr->addrs.daddr);
	pkt_len = ip_hdr->tot_len;
	is_tcp_ = 0;
	is_udp_ = 0;
	qh_ = qh;

	printf("Received packet of length = %d bytes\n", payload_len);
}
int Packet::ip_length() {
	return pkt_len;
}
int Packet::ip_header_length() {
	return 20;
	/*
	if (ipv4) {
		return ip->ihl * ip->version;
	}
	
	//TODO for ipv6
	return sizeof(struct iphdr);  //default length of ipv4
				    */
}

void *Packet::packet_start() {
	return payload;
}

void *Packet::l4_header_start() {
#ifdef IPV6
	if (ipv4)
#endif
		return payload + sizeof(struct iphdr);
#ifdef IPV6
	return payload + sizeof(struct ip6hdr);
#endif
}

int Packet::is_valid() {
	int ip_version;
	struct iphdr *ip_hdr;


	if (payload_len < sizeof(struct iphdr))
		return 0;
	ip_version = payload[0] >> 4;
	if (ip_version != 4)
		return 0;
	ip_hdr = (struct iphdr*)payload;
	if (ip_hdr->protocol == 6)
		is_tcp_ = 1;
	else if (ip_hdr->protocol == 20)
		is_udp_ = 1;

	return 1;
}

int Packet::verdict(int verdict) {
	return nfq_set_verdict(qh_, 0, verdict, 0, NULL);
}

char Packet::is_tcp() {
	return is_tcp_;
}

char Packet::is_udp() {
	return is_udp_;
}

/*
class PacketUDP: public Packet {
	public:
		explicit PacketUDP(struct nfq_q_handle *qh, struct nfq_data *nf_d);
		int parse();
		char match_sni(char *);
};
*/

class PacketTCP: public Packet {
	private:
		struct tcphdr *tcp;
		int tls_application;
		int l4_header_length();
		unsigned int tcp_seq;
	public:
		explicit PacketTCP(struct nfq_q_handle *qh, struct nfq_data *nf_d, FlowManager *nf_listener);
		~PacketTCP() override = default;
		int parse();
		char match_sni(char *stream);
		FlowKey make_flow_key() const;
		unsigned short source_port() const {
			return tcp->source;
		}
		unsigned short dest_port() const {
			return tcp->dest;
		}
};

PacketTCP::PacketTCP(struct nfq_q_handle *qh, struct nfq_data *nf_d, FlowManager *flow_m)
	: Packet(qh, nf_d, flow_m)
{
	tcp = (struct tcphdr*)l4_header_start();
}

FlowKey PacketTCP::make_flow_key() const {
    FlowKey key;

    key.ip1   = source_ip();
    key.ip2   = dest_ip();
    key.port1 = source_port();
    key.port2 = dest_port();
    key.protocol = 6;

    return key;
}


char PacketTCP::match_sni(char *stream) {

	return 0;
}

int PacketTCP::l4_header_length() {
	return tcp->doff * 8;
}

int PacketTCP::parse() {
	tcp = (struct tcphdr*)l4_header_start();
	if (tcp->doff < 4) {
		verdict(NF_ACCEPT);
		return 0;
	}
	if (ip_length() == (ip_header_length() + l4_header_length())) {
		verdict(NF_ACCEPT);
		return 0;
	}
	if (tcp->syn || tcp->fin || tcp->rst) {
		//packet to be PASS without further processing
		verdict(NF_ACCEPT);
		return 0;
	}

	unsigned char *tsl_hdr = (unsigned char *)l4_header_start() + l4_header_length();
	char tsl_content_type = tsl_hdr[0];
	if (tsl_content_type == 0x16 || tcp->psh) {
		tls_application = 1;
		tcp_seq = tcp->seq;
	}

	return 1;
}


Flow::Flow(const FlowKey& key)
    : key_(key),
      last_seen_ns_(0),
      protocol_identified_(false)
{
}

/*
class FlowUDP : public Flow {
	public:
		explicit FlowUDP(const FlowKey& key);

		void process_packet(const uint8_t* data,
                        size_t len,
                        uint64_t timestamp_ns) override;

	private:

		// UDP stream buffer (very simple)
		std::vector<uint8_t> stream_buffer_;

		// DPI
		void detect_protocol();
		void parse_quic();
		void parse_http();
};
*/

class FlowTCP : public Flow {
	public:
		explicit FlowTCP(const FlowKey& key);

		void process_packet(const uint8_t* data,
                        size_t len,
                        uint64_t timestamp_ns) override;

	private:
		bool syn_seen_;
		bool fin_seen_;
		bool rst_seen_;

		// TCP stream buffer (very simple)
		std::vector<uint8_t> stream_buffer_;

		// DPI
		void detect_protocol();
		void parse_tls();
		void parse_http();
};

FlowTCP::FlowTCP(const FlowKey& key)
    : Flow(key),
      syn_seen_(false),
      fin_seen_(false),
      rst_seen_(false)
{
}

void FlowTCP::process_packet(const uint8_t* data,
                             size_t len,
                             uint64_t timestamp_ns)
{
    last_seen_ns_ = timestamp_ns;

    if (len == 0)
        return;

    // Append payload to stream buffer
    stream_buffer_.insert(stream_buffer_.end(), data, data + len);

    if (!protocol_identified_) {
        detect_protocol();
    }

    if (protocol_identified_) {
        parse_tls();
        parse_http();
    }
}

void FlowTCP::detect_protocol()
{
    // Very naive detection
    if (stream_buffer_.size() < 5)
        return;

    // TLS record header: 0x16 0x03 0x01+
    if (stream_buffer_[0] == 0x16 &&
        stream_buffer_[1] == 0x03) {
        protocol_identified_ = true;
    }

    // HTTP methods
    static const char* methods[] = {
        "GET", "POST", "HEAD", "PUT", "DELETE"
    };

    for (auto m : methods) {
        if (std::memcmp(stream_buffer_.data(), m, std::strlen(m)) == 0) {
            protocol_identified_ = true;
            break;
        }
    }
}

void FlowTCP::parse_tls()
{
}

void FlowTCP::parse_http()
{
}


FlowManager::FlowManager()
{
	xdp_dpi_map_fd = bpf_obj_get(XDP_TRIE_MAP_PATH);
	xdp_flow_map_fd = bpf_obj_get(XDP_HASH_MAP_PATH);

	//if (xdp_trie_map_fd < 0 || xdp_hash_map_fd < 0)
	//TODO implementation exit progrma if xdp map not found 	

}

int FlowManager::update_xdp_map(unsigned int dest_ip) {
      //TODO call bpf_obj_update_elem()
      return true;
}

//Flow* FlowManager::get_create_flow(const FlowKey& key, uint64 ts)
Flow* FlowManager::get_create_flow(const FlowKey& key)
{

    auto it = flows_.find(key);
    if (it != flows_.end())
        return it->second.get();

    std::unique_ptr<Flow> flow;

//    if (key.protocol == 6)
        flow = std::make_unique<FlowTCP>(key);
//    else
//        flow = std::make_unique<FlowUDP>(key);

    Flow* ret = flow.get();
    flows_.insert(std::make_pair(key, std::move(flow)));
    return ret;
}


std::unique_ptr<Packet> handle_packet(struct nfq_q_handle *qh, struct nfq_data *nf_d, unsigned char *payload, FlowManager *flow_m) {
	struct iphdr *iph = (struct iphdr*) payload;
	char proto;

	if (iph->version == 4) {
		proto = iph->protocol;

//		if (proto == 6)
			return std::make_unique<PacketTCP>(qh, nf_d, flow_m);
//		else if (proto == 20)
//			return std::make_unique<PacketUDP>(qh, nf_d, flow_m);

	}

	return NULL;
}


class dpi_nfqueue_listener{
	private:
		unsigned long queue_number;
		int fd;
		struct nfq_handle *queue;
		struct nfq_q_handle *queue_handle;
		FlowManager *flow_m;

	public:
		explicit dpi_nfqueue_listener(unsigned long q_num);
		int init();
		void run();
	private:
		static int callback(struct nfq_q_handle *qh,
                                struct nfgenmsg *msg,
                                struct nfq_data *nf_d,
                                void *nf_listener) {
			unsigned char *payload = NULL;
			int payload_len;
		//	struct nfqnl_msg_packet_hdr *qh;
			FlowManager *flow_hdlr = (FlowManager *)nf_listener;
			Flow *flow;

			payload_len = nfq_get_payload(nf_d, &payload);
		//	qh = nfq_get_msg_packet_hdr(nf_d);
			if (!(payload_len < sizeof(struct iphdr)) || !qh) {
				std::unique_ptr<Packet> pkt = handle_packet(qh, nf_d, payload, flow_hdlr);
				pkt->parse();
				flow = flow_hdlr->get_create_flow(pkt->make_flow_key());
			}

			return nfq_set_verdict(qh, 0, NF_ACCEPT, 0, NULL);
		}
};

dpi_nfqueue_listener::dpi_nfqueue_listener(unsigned long qnum) 
	: flow_m(&FlowManager::instance())
{
	queue_number = qnum;
}

int dpi_nfqueue_listener::init() {
	int ret;
	//flow_m = FlowManager::instance();

	queue = nfq_open();
	if (!queue) {
		return false;
	}
	ret = nfq_unbind_pf(queue, AF_INET);
	if (ret < -1)
		return false;
	ret = nfq_bind_pf(queue, AF_INET);
	if (ret < -1)
		return false;

	queue_handle = nfq_create_queue(queue, queue_number, &dpi_nfqueue_listener::callback, (void *)flow_m);
	if (!queue_handle) {
		return false;
	}
	if (nfq_set_mode(queue_handle, NFQNL_COPY_PACKET, 2048) < -1)
		return false;
	return true;
}

void dpi_nfqueue_listener::run()
{
    char buf[4096] __attribute__((aligned));
    int ret;

    while ((ret = recv(fd, buf, sizeof(buf), 0)) >= 0) {
        nfq_handle_packet(queue, buf, ret);
    }
}



int main()
{
	dpi_nfqueue_listener dpi_nfq_lstnr(100);

	dpi_nfq_lstnr.init();
	dpi_nfq_lstnr.run();
	return 0;
}
