#include <App.hpp>

#define ENET_IMPLEMENTATION
#include <enet.h>

#include <vector>
#include <thread>
#include <random>
#include <chrono>
#include <vector>

struct NetPacket
{
    char header[8]{ "" };
    u32 size{ 0 };
    char* data{ nullptr };
};

struct NetClient
{
    ENetHost* client{ nullptr };
    ENetPeer* peer{ nullptr };
};

struct State
{
    NetRelayFn relayFn{ nullptr };

    ENetHost* server{ nullptr };
    std::vector<ENetPeer*> peers;

    std::vector<NetClient> clients;

    std::vector<NetPacket> packets{};
};

static State g_state{};

static ENetPacket* net_create_packet(const NetPacket& packet, NetPacketMode mode)
{
    int totalSize = sizeof(packet.header) + sizeof(packet.size) + packet.size;
    char* buffer = new char[totalSize];

    memcpy(buffer, packet.header, sizeof(packet.header));
    memcpy(buffer + sizeof(packet.header), &packet.size, sizeof(int));
    memcpy(buffer + sizeof(packet.header) + sizeof(int), packet.data, packet.size);

    enet_uint32 flags = (enet_uint32)mode;
    ENetPacket* enetPacket = enet_packet_create(buffer, totalSize, flags);

    delete[] buffer;
    return enetPacket;
}

static NetPacket net_receive_packet(ENetPacket* packet)
{
    NetPacket received{};

    // Ensure packet is large enough to contain `header` and `size`
    if (packet->dataLength >= sizeof(NetPacket::header) + sizeof(u32))
    {
        memcpy(received.header, packet->data, sizeof(received.header));
        memcpy(&received.size, (char*)packet->data + sizeof(received.header), sizeof(u32));

        if (received.size > 0 && packet->dataLength >= sizeof(received.header) + sizeof(u32) + received.size)
        {
            received.data = new char[received.size];
            memcpy(received.data, (char*)packet->data + sizeof(received.header) + sizeof(u32), received.size);
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
        g_state.peers[e.peer->incomingPeerID] = e.peer;
    }
    else
    {
        LOGD("Client connected!");
    }

    g_state.relayFn(server, client, (u32)NetEvent::CONNECT, (u32)e.peer->incomingPeerID, e.channelID, -1);
}

static void net_receive(const ENetEvent& e, bool server, u32 client)
{
    auto received = net_receive_packet(e.packet);

    if (received.data != nullptr)
    {
        g_state.packets.emplace_back(received);
        g_state.relayFn(server, client, (u32)NetEvent::RECEIVE, (u32)e.peer->incomingPeerID, e.channelID, (u32)(g_state.packets.size() - 1));
    }

    enet_packet_destroy(e.packet);
}

static void net_disconnect(const ENetEvent& e, bool server, u32 client)
{
    g_state.relayFn(server, client, (u32)NetEvent::DISCONNECT, (u32)e.peer->incomingPeerID, e.channelID, -1);

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
    g_state.relayFn(server, client, (u32)NetEvent::TIMEOUT, (u32)e.peer->incomingPeerID, e.channelID, -1);

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

bool App::NetInitialize(NetRelayFn relayFn)
{
    if (enet_initialize() != 0)
    {
        LOGE("Failed to initialize ENet.");
        return false;
    }

    g_state.relayFn = relayFn;
    return true;
}

void App::NetShutdown()
{
    NetReload();
    enet_deinitialize();
}

void App::NetReload()
{
    for (u32 i = 0; i < g_state.clients.size(); i++)
        NetDisconnectClient(i);

    if (NetIsServer())
        NetStopServer();
}

void App::NetStartServer(const char* ip, u32 port, u32 peerCount, u32 channelLimit)
{
    if (NetIsServer())
        return;

    ENetAddress address{};

    if (std::strcmp(ip, "any") == 0)
        address.host = ENET_HOST_ANY;
    else
        enet_address_set_host(&address, ip);
    address.port = port;

    g_state.server = enet_host_create(&address, peerCount, channelLimit, 0, 0);
    if (g_state.server == nullptr)
    {
        LOGE("Failed to create an ENet server.");
    }
    
    g_state.peers.clear();
    g_state.peers.resize(peerCount);
    LOGD("Server successfully started on: %d", address.port);
}

void App::NetStopServer()
{
    if (!NetIsServer())
        return;

    enet_host_destroy(g_state.server);
    g_state.server = nullptr;
}

u32 App::NetConnectClient(const char* ip, u32 port, u32 peerCount, u32 channelLimit)
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
    g_state.clients.emplace_back(client);
    return (u32)g_state.clients.size() - 1;
}

void App::NetDisconnectClient(u32 client)
{
    if (!NetIsClient(client))
        return;

    auto& c = g_state.clients[client];
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

bool App::NetIsServer() { return g_state.server != nullptr; }

bool App::NetIsClient(u32 client)
{
    if (client >= g_state.clients.size())
        return false;
    const auto& c = g_state.clients[client];
    return c.client != nullptr && c.peer != nullptr;
}

u32 App::NetCreatePacket(const char* id, u32 size)
{
    NetPacket packet{};
    auto id_len = strnlen(id, sizeof(packet.header)); // Get actual length up to 8 chars
    memcpy(packet.header, id, id_len); // Copy only the actual data
    packet.size = size;
    packet.data = new char[size];
    g_state.packets.emplace_back(std::move(packet));
    return (u32)(g_state.packets.size() - 1);
}

void App::NetBroadcast(u32 packet, u32 mode)
{
    // Only servers can broadcast
    if (!NetIsServer())
    {
        LOGW("Attempting to broadcast packet without running server!");
        return;
    }

    for (auto peer : g_state.peers)
    {
        if (peer != nullptr)
            enet_peer_send(peer, 0, net_create_packet(g_state.packets[packet], (NetPacketMode)mode));
    }
    enet_host_flush(g_state.server);
}

void App::NetSend(u32 client, u32 packet, u32 mode)
{
    // Ensure we have a valid connection
    if (!NetIsClient(client))
    {
        LOGW("Attempting to send packet with invalid client id: %d", client);
        return;
    }

    // Send to the peer
    const auto& c = g_state.clients[client];
    enet_peer_send(c.peer, 0, net_create_packet(g_state.packets[packet], (NetPacketMode)mode));
    enet_host_flush(c.client);
}

void App::NetPollEvents()
{
    if (g_state.server)
    {
        ENetEvent e{};
        while (enet_host_service(g_state.server, &e, 0) > 0)
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
    for (const auto& client : g_state.clients)
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
    for (const auto& p : g_state.packets)
        delete[] p.data;
    g_state.packets.clear();
}

bool App::NetGetBool(u32 packet, u32 offset)
{
    auto& p = g_state.packets[packet];
    if (offset + 1 > p.size)
    {
        LOGW("Attempting to read past the packet boundary!");
        return false;
    }

    return p.data[offset] != 0;
}

u32 App::NetGetUInt(u32 packet, u32 offset)
{
    auto& p = g_state.packets[packet];
    if (offset + sizeof(u32) > p.size)
    {
        LOGW("Attempting to read past the packet boundary!");
        return 0;
    }

    u32 value;
    std::memcpy(&value, p.data + offset, sizeof(u32));
    return value;
}

i32 App::NetGetInt(u32 packet, u32 offset)
{
    auto& p = g_state.packets[packet];
    if (offset + sizeof(i32) > p.size)
    {
        LOGW("Attempting to read past the packet boundary!");
        return 0;
    }

    i32 value;
    std::memcpy(&value, p.data + offset, sizeof(i32));
    return value;
}

f32 App::NetGetFloat(u32 packet, u32 offset)
{
    auto& p = g_state.packets[packet];
    if (offset + sizeof(f32) > p.size)
    {
        LOGW("Attempting to read past the packet boundary!");
        return 0.0f;
    }

    f32 value;
    std::memcpy(&value, p.data + offset, sizeof(f32));
    return value;
}

f64 App::NetGetDouble(u32 packet, u32 offset)
{
    auto& p = g_state.packets[packet];
    if (offset + sizeof(f64) > p.size)
    {
        LOGW("Attempting to read past the packet boundary!");
        return 0.0;
    }

    f64 value;
    std::memcpy(&value, p.data + offset, sizeof(f64));
    return value;
}

void App::NetSetBool(u32 packet, u32 offset, bool v)
{
    auto& p = g_state.packets[packet];
    if (offset + 1 > p.size)
    {
        LOGW("Attempting to write past the packet boundary!");
        return;
    }

    p.data[offset] = v ? 1 : 0;
}

void App::NetSetUInt(u32 packet, u32 offset, u32 v)
{
    auto& p = g_state.packets[packet];
    if (offset + sizeof(u32) > p.size)
    {
        LOGW("Attempting to write past the packet boundary!");
        return;
    }

    std::memcpy(p.data + offset, &v, sizeof(u32));
}

void App::NetSetInt(u32 packet, u32 offset, i32 v)
{
    auto& p = g_state.packets[packet];
    if (offset + sizeof(i32) > p.size)
    {
        LOGW("Attempting to write past the packet boundary!");
        return;
    }

    std::memcpy(p.data + offset, &v, sizeof(i32));
}

void App::NetSetFloat(u32 packet, u32 offset, f32 v)
{
    auto& p = g_state.packets[packet];
    if (offset + sizeof(f32) > p.size)
    {
        LOGW("Attempting to write past the packet boundary!");
        return;
    }

    std::memcpy(p.data + offset, &v, sizeof(f32));
}

void App::NetSetDouble(u32 packet, u32 offset, f64 v)
{
    auto& p = g_state.packets[packet];
    if (offset + sizeof(f64) > p.size)
    {
        LOGW("Attempting to write past the packet boundary!");
        return;
    }

    std::memcpy(p.data + offset, &v, sizeof(f64));
}