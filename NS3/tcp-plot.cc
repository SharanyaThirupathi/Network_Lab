#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include <fstream>

using namespace ns3;

std::ofstream cwndFile;

void
CwndChange(uint32_t oldCwnd, uint32_t newCwnd)
{
    cwndFile << Simulator::Now().GetSeconds()
             << " "
             << newCwnd
             << std::endl;

    std::cout << "Time: "
              << Simulator::Now().GetSeconds()
              << " s   CWND: "
              << newCwnd
              << " bytes"
              << std::endl;
}

void
ConnectCwndTrace()
{
    Config::ConnectWithoutContext(
        "/NodeList/0/$ns3::TcpL4Protocol/SocketList/*/CongestionWindow",
        MakeCallback(&CwndChange));

    std::cout << "TCP Congestion Window Trace Connected"
              << std::endl;
}

int
main()
{
    cwndFile.open("cwnd.dat");

    std::cout << "====================================" << std::endl;
    std::cout << " TCP CONGESTION CONTROL WITH PLOTTING" << std::endl;
    std::cout << "====================================" << std::endl;

    NodeContainer nodes;
    nodes.Create(2);

    std::cout << "\nNodes Created" << std::endl;
    std::cout << "Node 0 - Sender" << std::endl;
    std::cout << "Node 1 - Receiver" << std::endl;

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("2Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("10ms"));

    NetDeviceContainer devices;
    devices = p2p.Install(nodes);

    std::cout << "\nPoint-to-Point Link Created" << std::endl;
    std::cout << "Data Rate: 2Mbps" << std::endl;
    std::cout << "Delay: 10ms" << std::endl;

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces;
    interfaces = address.Assign(devices);

    std::cout << "\nIP Addresses" << std::endl;
    std::cout << "Sender   : "
              << interfaces.GetAddress(0) << std::endl;

    std::cout << "Receiver : "
              << interfaces.GetAddress(1) << std::endl;

    uint16_t port = 5000;

    PacketSinkHelper sink(
        "ns3::TcpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), port));

    ApplicationContainer sinkApp;
    sinkApp = sink.Install(nodes.Get(1));

    sinkApp.Start(Seconds(1.0));
    sinkApp.Stop(Seconds(20.0));

    BulkSendHelper source(
        "ns3::TcpSocketFactory",
        InetSocketAddress(interfaces.GetAddress(1), port));

    source.SetAttribute("MaxBytes", UintegerValue(0));
    source.SetAttribute("SendSize", UintegerValue(1024));

    ApplicationContainer sourceApp;
    sourceApp = source.Install(nodes.Get(0));

    sourceApp.Start(Seconds(2.0));
    sourceApp.Stop(Seconds(20.0));

    Simulator::Schedule(
        Seconds(2.1),
        &ConnectCwndTrace);

    std::cout << "\nStarting Simulation..." << std::endl;

    Simulator::Stop(Seconds(20.0));

    Simulator::Run();

    Simulator::Destroy();

    cwndFile.close();

    std::cout << "\n====================================" << std::endl;
    std::cout << " TCP Simulation Completed" << std::endl;
    std::cout << "====================================" << std::endl;

    return 0;
}
