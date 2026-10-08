#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
    CommandLine cmd;
    cmd.Parse (argc, argv);

    std::cout << "========================================" << std::endl;
    std::cout << "       TCP SIMULATION USING NS-3" << std::endl;
    std::cout << "========================================" << std::endl;

    // 1. Create Nodes
    NodeContainer nodes;
    nodes.Create (2);

    std::cout << "\n1. Nodes Created" << std::endl;
    std::cout << "Node 0 : Sender" << std::endl;
    std::cout << "Node 1 : Receiver" << std::endl;

    // 2. Setup Point-to-Point Link
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute (
        "DataRate",
        StringValue ("5Mbps"));

    pointToPoint.SetChannelAttribute (
        "Delay",
        StringValue ("2ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install (nodes);

    std::cout << "\n2. Point-to-Point Link Created" << std::endl;
    std::cout << "Data Rate : 5 Mbps" << std::endl;
    std::cout << "Delay     : 2 ms" << std::endl;

    // 3. Install Internet Stack
    InternetStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase (
        "10.1.1.0",
        "255.255.255.0");

    Ipv4InterfaceContainer interfaces;
    interfaces = address.Assign (devices);

    std::cout << "\n3. IP Addresses Assigned" << std::endl;
    std::cout << "Sender   : "
              << interfaces.GetAddress (0)
              << std::endl;

    std::cout << "Receiver : "
              << interfaces.GetAddress (1)
              << std::endl;

    // 4. Set TCP Congestion Control
    Config::SetDefault (
        "ns3::TcpL4Protocol::SocketType",
        StringValue ("ns3::TcpCubic"));

    std::cout << "\n4. TCP Congestion Control" << std::endl;
    std::cout << "Algorithm : TCP CUBIC" << std::endl;

    // 5. Setup Receiver
    uint16_t port = 8080;

    Address localAddress (
        InetSocketAddress (
            Ipv4Address::GetAny (),
            port));

    PacketSinkHelper packetSinkHelper (
        "ns3::TcpSocketFactory",
        localAddress);

    ApplicationContainer sinkApp =
        packetSinkHelper.Install (nodes.Get (1));

    sinkApp.Start (Seconds (1.0));
    sinkApp.Stop (Seconds (10.0));

    std::cout << "\n5. Receiver Configured" << std::endl;
    std::cout << "Port : " << port << std::endl;
    std::cout << "Start Time : 1 second" << std::endl;
    std::cout << "Stop Time  : 10 seconds" << std::endl;

    // 6. Setup Sender
    InetSocketAddress remoteAddress (
        interfaces.GetAddress (1),
        port);

    OnOffHelper clientHelper (
        "ns3::TcpSocketFactory",
        remoteAddress);

    clientHelper.SetAttribute (
        "DataRate",
        StringValue ("5Mbps"));

    clientHelper.SetAttribute (
        "PacketSize",
        StringValue ("1024"));

    ApplicationContainer clientApp =
        clientHelper.Install (nodes.Get (0));

    clientApp.Start (Seconds (2.0));
    clientApp.Stop (Seconds (9.0));

    std::cout << "\n6. Sender Configured" << std::endl;
    std::cout << "Data Rate  : 5 Mbps" << std::endl;
    std::cout << "Packet Size: 1024 bytes" << std::endl;
    std::cout << "Start Time : 2 seconds" << std::endl;
    std::cout << "Stop Time  : 9 seconds" << std::endl;

    // 7. Run Simulation
    std::cout << "\n7. Starting TCP Simulation..." << std::endl;

    Simulator::Stop (Seconds (10.0));
    Simulator::Run ();

    std::cout << "\n8. Simulation Completed Successfully" << std::endl;

    Simulator::Destroy ();

    std::cout << "\n========================================" << std::endl;
    std::cout << "       TCP SIMULATION FINISHED" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
