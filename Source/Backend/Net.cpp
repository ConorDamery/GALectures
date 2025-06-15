#include <App.hpp>

#define ENET_IMPLEMENTATION
#include <enet.h>

#include <cstring>
#include <thread>
#include <random>
#include <chrono>

using namespace GASandbox;

using NetcodeFn = void (*)(bool, u32, eNetEvent, u16, u32, u32);

struct NetPacket
{
    u32 id{ 0 };
    u32 size{ 0 };
    char* data{ nullptr };
};

struct NetClient
{
    ENetHost* client{ nullptr };
    ENetPeer* peer{ nullptr };
};

struct NetGlobal
{
    NetcodeFn netcodeFn{ nullptr };

    ENetHost* server{ nullptr };
    list<ENetPeer*> peers;

    list<NetClient> clients;

    list<NetPacket> packets{};
};

static NetGlobal g{};

static ENetPacket* net_create_packet(const NetPacket& packet, eNetPacketMode mode)
{
    int totalSize = sizeof(packet.id) + sizeof(packet.size) + packet.size;
    char* buffer = new char[totalSize];

    u32 offset = 0;
    memcpy(buffer, &packet.id, sizeof(packet.id));
    offset += sizeof(packet.id);
    memcpy(buffer + offset, &packet.size, sizeof(packet.size));
    offset += sizeof(packet.size);
    memcpy(buffer + offset, packet.data, packet.size);

    enet_uint32 flags = (enet_uint32)mode;
    ENetPacket* enetPacket = enet_packet_create(buffer, totalSize, flags);

    delete[] buffer;
    return enetPacket;
}

static NetPacket net_receive_packet(ENetPacket* packet)
{
    NetPacket received{};

    // Ensure packet is large enough to contain `id` and `size`
    const size_type headerSize = sizeof(NetPacket::id) + sizeof(NetPacket::size);
    if (packet->dataLength >= headerSize)
    {
        u32 offset = 0;
        memcpy(&received.id, packet->data, sizeof(received.id));
        offset += sizeof(received.id);
        memcpy(&received.size, (char*)packet->data + offset, sizeof(received.size));
        offset += sizeof(received.size);

        if (received.size > 0 && packet->dataLength >= headerSize + received.size)
        {
            received.data = new char[received.size];
            memcpy(received.data, (char*)packet->data + offset, received.size);
        }
        else
        {
            received.data = nullptr;
        }
    }
    else
    {
        LOGE("Received packet too small! Size: %d", packet->dataLength);
    }

    return received;
}

static void net_connect(const ENetEvent& e, bool server, u32 client)
{
    if (server)
    {
        LOGD("A new peer with ID %u connected from ::1:%u.", e.peer->incomingPeerID, e.peer->address.port);
        g.peers[e.peer->incomingPeerID] = e.peer;
    }
    else
    {
        LOGD("Client connected!");
    }

    g.netcodeFn(server, client, eNetEvent::CONNECT, e.peer->incomingPeerID, e.channelID, -1);
}

static void net_receive(const ENetEvent& e, bool server, u32 client)
{
    auto received = net_receive_packet(e.packet);

    if (received.data != nullptr)
    {
        g.packets.emplace_back(received);
        g.netcodeFn(server, client, eNetEvent::RECEIVE, e.peer->incomingPeerID, e.channelID, (u32)(g.packets.size() - 1));
    }

    enet_packet_destroy(e.packet);
}

static void net_disconnect(const ENetEvent& e, bool server, u32 client)
{
    g.netcodeFn(server, client, eNetEvent::DISCONNECT, e.peer->incomingPeerID, e.channelID, -1);

    if (server)
    {
        LOGD("Peer with ID %u disconnected.", e.peer->incomingPeerID);
        
    }
    else
    {
        LOGD("Client disconnected.");
        App::NetDisconnectClient((u32)e.peer->incomingPeerID);
    }
}

static void net_timeout(const ENetEvent& e, bool server, u32 client)
{
    g.netcodeFn(server, client, eNetEvent::TIMEOUT, e.peer->incomingPeerID, e.channelID, -1);

    if (server)
    {
        LOGD("Client %u timeout.", e.peer->incomingPeerID);
    }
    else
    {
        LOGD("Client disconnected.");
    }

    net_disconnect(e, server, client);
}

static bool net_validate_packet(u32 packet)
{
    if (packet >= g.packets.size())
    {
        LOGW("Attempting to access invalid packet!");
        return false;
    }
    return true;
}

static bool net_guard_packet(u32 packet_size, u32 offset, u32 size)
{
    if (offset + size > packet_size)
    {
        LOGW("Attempting to access past the packet boundary!");
        return false;
    }
    return true;
}

bool App::NetInitialize(const sAppConfig& config)
{
    if (enet_initialize() != 0)
    {
        LOGE("Failed to initialize ENet.");
        return false;
    }

    g.netcodeFn = App::CodeNetcode;
    return true;
}

void App::NetShutdown()
{
    NetReload();
    enet_deinitialize();
}

void App::NetReload()
{
    for (u32 i = 0; i < g.clients.size(); i++)
        NetDisconnectClient(i);

    if (NetIsServer())
        NetStopServer();

    // Net API
    CodeBindMethod("app", "App", true, "netStartServer(_,_,_,_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 4);
            NetStartServer(CodeGetSlotString(vm, 1), CodeGetSlotUInt(vm, 2), CodeGetSlotUInt(vm, 3), CodeGetSlotUInt(vm, 4));
        });

    CodeBindMethod("app", "App", true, "netStopServer()",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 1);
            NetStopServer();
        });

    CodeBindMethod("app", "App", true, "netConnectClient(_,_,_,_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 4);
            u32 client = NetConnectClient(CodeGetSlotString(vm, 1), CodeGetSlotUInt(vm, 2), CodeGetSlotUInt(vm, 3), CodeGetSlotUInt(vm, 4));
            CodeSetSlotUInt(vm, 0, client);
        });

    CodeBindMethod("app", "App", true, "netDisconnectClient(_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 1);
            NetDisconnectClient(CodeGetSlotUInt(vm, 1));
        });

    CodeBindMethod("app", "App", true, "netMakeUuid()",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 1);
            CodeSetSlotUInt(vm, 0, NetMakeUUID());
        });

    CodeBindMethod("app", "App", true, "netIsServer()",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 1);
            CodeSetSlotBool(vm, 0, NetIsServer());
        });

    CodeBindMethod("app", "App", true, "netIsClient(_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 1);
            CodeSetSlotBool(vm, 0, NetIsClient(CodeGetSlotUInt(vm, 1)));
        });

    CodeBindMethod("app", "App", true, "netCreatePacket(_,_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 2);
            auto packet = NetCreatePacket(CodeGetSlotUInt(vm, 1), CodeGetSlotUInt(vm, 2));
            CodeSetSlotUInt(vm, 0, packet);
        });

    CodeBindMethod("app", "App", true, "netPacketId(_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 1);
            auto id = NetPacketId(CodeGetSlotUInt(vm, 1));
            CodeSetSlotUInt(vm, 0, id);
        });

    CodeBindMethod("app", "App", true, "netBroadcast(_,_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 2);
            NetBroadcast(CodeGetSlotUInt(vm, 1), (eNetPacketMode)CodeGetSlotUInt(vm, 2));
        });

    CodeBindMethod("app", "App", true, "netSend(_,_,_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 3);
            NetSend(CodeGetSlotUInt(vm, 1), CodeGetSlotUInt(vm, 2), (eNetPacketMode)CodeGetSlotUInt(vm, 3));
        });

#define SCRIPT_NET_GET(Type)																			\
	CodeBindMethod("app", "App", true, "netGet"#Type"(_,_)",											\
	[](sCodeVM* vm)																					    \
		{																								\
			CodeEnsureSlots(vm, 2);																		\
			CodeSetSlot##Type(vm, 0, NetGet##Type(CodeGetSlotUInt(vm, 1), CodeGetSlotUInt(vm, 2)));	    \
		});

    SCRIPT_NET_GET(Bool);
    SCRIPT_NET_GET(UInt);
    SCRIPT_NET_GET(Int);
    SCRIPT_NET_GET(Float);
    SCRIPT_NET_GET(Double);

    CodeBindMethod("app", "App", true, "netGetString(_,_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 2);
            CodeSetSlotString(vm, 0, NetGetString(CodeGetSlotUInt(vm, 1), CodeGetSlotUInt(vm, 2)));
        });

#define SCRIPT_NET_SET(Type)																			\
	CodeBindMethod("app", "App", true, "netSet"#Type"(_,_,_)",											\
	[](sCodeVM* vm)																					    \
		{																								\
			CodeEnsureSlots(vm, 3);																		\
			NetSet##Type(CodeGetSlotUInt(vm, 1), CodeGetSlotUInt(vm, 2), CodeGetSlot##Type(vm, 3));	    \
		});

    SCRIPT_NET_SET(Bool);
    SCRIPT_NET_SET(UInt);
    SCRIPT_NET_SET(Int);
    SCRIPT_NET_SET(Float);
    SCRIPT_NET_SET(Double);

    CodeBindMethod("app", "App", true, "netSetString(_,_,_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 3);
            NetSetString(CodeGetSlotUInt(vm, 1), CodeGetSlotUInt(vm, 2), CodeGetSlotString(vm, 3));
        });
}

void App::NetStartServer(cstring ip, u32 port, u32 peerCount, u32 channelLimit)
{
    if (NetIsServer())
        return;

    ENetAddress address{};

    if (strcmp(ip, "any") == 0)
        address.host = ENET_HOST_ANY;
    else
        enet_address_set_host(&address, ip);
    address.port = port;

    g.server = enet_host_create(&address, peerCount, channelLimit, 0, 0);
    if (g.server == nullptr)
    {
        LOGE("Failed to create an ENet server.");
    }
    
    g.peers.clear();
    g.peers.resize(peerCount);
    LOGD("Server successfully started on: %d", address.port);
}

void App::NetStopServer()
{
    if (!NetIsServer())
        return;

    enet_host_destroy(g.server);
    g.server = nullptr;
}

u32 App::NetConnectClient(cstring ip, u32 port, u32 peerCount, u32 channelLimit)
{
    NetClient client{};
    client.client = enet_host_create(nullptr, peerCount, channelLimit, 0, 0);
    if (client.client == nullptr)
    {
        LOGE("Failed to start client.");
        return -1;
    }

    ENetAddress address{};
    enet_address_set_host(&address, ip);
    address.port = port;

    client.peer = enet_host_connect(client.client, &address, 2, 0);
    if (!client.peer)
    {
        LOGE("No available peers for connection.");
        enet_host_destroy(client.client);
        client.client = nullptr;
        return -1;
    }

    LOGD("Client attempting to connect...");
    g.clients.emplace_back(client);
    return (u32)g.clients.size() - 1;
}

void App::NetDisconnectClient(u32 client)
{
    if (!NetIsClient(client))
        return;

    auto& c = g.clients[client];
    enet_peer_disconnect(c.peer, 0);
    enet_peer_reset(c.peer);
    enet_host_destroy(c.client);
    c.client = nullptr;
    c.peer = nullptr;
}

u32 App::NetMakeUUID()
{
    auto timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::random_device rd;
    std::mt19937 rng(rd());
    u64 uuid64 = timestamp ^ rng();
    return static_cast<u32>(uuid64) ^ static_cast<u32>(uuid64 >> 32);
}

bool App::NetIsServer() { return g.server != nullptr; }

bool App::NetIsClient(u32 client)
{
    if (client >= g.clients.size())
        return false;
    const auto& c = g.clients[client];
    return c.client != nullptr && c.peer != nullptr;
}

u32 App::NetCreatePacket(u32 id, u32 size)
{
    NetPacket packet{};
    memcpy(&packet.id, &id, sizeof(packet.id)); // Copy only the actual data
    packet.size = size;
    packet.data = new char[size];
    g.packets.emplace_back(std::move(packet));
    return (u32)(g.packets.size() - 1);
}

u32 App::NetPacketId(u32 packet)
{
    if (!net_validate_packet(packet))
        return -1;
    return g.packets[packet].id;
}

void App::NetBroadcast(u32 packet, eNetPacketMode mode)
{
    if (!net_validate_packet(packet))
        return;

    // Only servers can broadcast
    if (!NetIsServer())
    {
        LOGW("Attempting to broadcast packet without running server!");
        return;
    }

    for (auto peer : g.peers)
    {
        if (peer != nullptr)
            enet_peer_send(peer, 0, net_create_packet(g.packets[packet], mode));
    }
    enet_host_flush(g.server);
}

void App::NetSend(u32 client, u32 packet, eNetPacketMode mode)
{
    if (!net_validate_packet(packet))
        return;

    // Ensure we have a valid connection
    if (!NetIsClient(client))
    {
        LOGW("Attempting to send packet with invalid client id: %d", client);
        return;
    }

    // Send to the peer
    const auto& c = g.clients[client];
    enet_peer_send(c.peer, 0, net_create_packet(g.packets[packet], mode));
    enet_host_flush(c.client);
}

void App::NetPollEvents()
{
    if (g.server)
    {
        ENetEvent e{};
        while (enet_host_service(g.server, &e, 0) > 0)
        {
            switch (e.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                net_connect(e, true, -1);
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                net_receive(e, true, -1);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                net_disconnect(e, true, -1);
                break;

            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                net_timeout(e, true, -1);
                break;
            }
        }
    }

    u32 clientIdx = 0;
    for (const auto& client : g.clients)
    {
        ENetEvent e{};
        while (enet_host_service(client.client, &e, 0) > 0)
        {
            switch (e.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                net_connect(e, false, clientIdx);
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                net_receive(e, false, clientIdx);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                net_disconnect(e, false, clientIdx);
                break;

            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                net_timeout(e, false, clientIdx);
                break;
            }
        }

        clientIdx++;
    }

    // Free memory
    for (const auto& p : g.packets)
        delete[] p.data;
    g.packets.clear();
}

bool App::NetGetBool(u32 packet, u32 offset)
{
    if (!net_validate_packet(packet))
        return false;
    
    auto& p = g.packets[packet];
    if (!net_guard_packet(p.size, offset, 1))
        return false;

    return p.data[offset] != 0;
}

u32 App::NetGetUInt(u32 packet, u32 offset)
{
    if (!net_validate_packet(packet))
        return 0;

    auto& p = g.packets[packet];
    if (!net_guard_packet(p.size, offset, sizeof(u32)))
        return 0;

    u32 value;
    std::memcpy(&value, p.data + offset, sizeof(u32));
    return value;
}

i32 App::NetGetInt(u32 packet, u32 offset)
{
    if (!net_validate_packet(packet))
        return 0;

    auto& p = g.packets[packet];
    if (!net_guard_packet(p.size, offset, sizeof(i32)))
        return 0;

    i32 value;
    std::memcpy(&value, p.data + offset, sizeof(i32));
    return value;
}

f32 App::NetGetFloat(u32 packet, u32 offset)
{
    if (!net_validate_packet(packet))
        return 0;

    auto& p = g.packets[packet];
    if (!net_guard_packet(p.size, offset, sizeof(f32)))
        return 0;

    f32 value;
    std::memcpy(&value, p.data + offset, sizeof(f32));
    return value;
}

f64 App::NetGetDouble(u32 packet, u32 offset)
{
    if (!net_validate_packet(packet))
        return 0;

    auto& p = g.packets[packet];
    if (!net_guard_packet(p.size, offset, sizeof(f64)))
        return 0;

    f64 value;
    std::memcpy(&value, p.data + offset, sizeof(f64));
    return value;
}

cstring App::NetGetString(u32 packet, u32 offset)
{
    if (!net_validate_packet(packet))
        return "";

    auto& p = g.packets[packet];
    if (!net_guard_packet(p.size, offset, p.size - offset))
        return "";

    char* str = reinterpret_cast<char*>(p.data + offset);
    size_type maxLen = (size_type)p.size - offset;

    cstring end = static_cast<cstring>(std::memchr(str, '\0', maxLen));
    if (end == nullptr)
    {
        LOGW("String read attempt without null termination!");
        return "";
    }

    return str;
}

void App::NetSetBool(u32 packet, u32 offset, bool v)
{
    if (!net_validate_packet(packet))
        return;

    auto& p = g.packets[packet];
    if (!net_guard_packet(p.size, offset, 1))
        return;

    p.data[offset] = v ? 1 : 0;
}

void App::NetSetUInt(u32 packet, u32 offset, u32 v)
{
    if (!net_validate_packet(packet))
        return;

    auto& p = g.packets[packet];
    if (!net_guard_packet(p.size, offset, sizeof(u32)))
        return;

    std::memcpy(p.data + offset, &v, sizeof(u32));
}

void App::NetSetInt(u32 packet, u32 offset, i32 v)
{
    if (!net_validate_packet(packet))
        return;

    auto& p = g.packets[packet];
    if (!net_guard_packet(p.size, offset, sizeof(i32)))
        return;

    std::memcpy(p.data + offset, &v, sizeof(i32));
}

void App::NetSetFloat(u32 packet, u32 offset, f32 v)
{
    if (!net_validate_packet(packet))
        return;

    auto& p = g.packets[packet];
    if (!net_guard_packet(p.size, offset, sizeof(f32)))
        return;

    std::memcpy(p.data + offset, &v, sizeof(f32));
}

void App::NetSetDouble(u32 packet, u32 offset, f64 v)
{
    if (!net_validate_packet(packet))
        return;

    auto& p = g.packets[packet];
    if (!net_guard_packet(p.size, offset, sizeof(f64)))
        return;

    std::memcpy(p.data + offset, &v, sizeof(f64));
}

void App::NetSetString(u32 packet, u32 offset, cstring v)
{
    if (!net_validate_packet(packet))
        return;

    if (v == nullptr)
    {
        LOGW("Attempting to write a null string to packet!");
        return;
    }

    auto& p = g.packets[packet];

    // Search for null terminator within packet bounds
    cstring end = (cstring)std::memchr(v, '\0', p.size - static_cast<size_type>(offset) - 1);
    u32 length = end ? (end - v) : (p.size - static_cast<size_type>(offset) - 1); // Truncate if too long

    if (!net_guard_packet(p.size, offset, length))
        return;

    std::memcpy(p.data + offset, v, length);
}