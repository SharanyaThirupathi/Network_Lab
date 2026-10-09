#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpComparison");

// Callback function to trace Congestion Window (Cwnd) changes
static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t flowId, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << newCwnd << std::endl;
}

// Function to connect the TraceSource to the CwndChange sink
void
TraceCwnd (uint32_t flowId, Ptr<OutputStreamWrapper> stream)
{
  Config::ConnectWithoutContext ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/CongestionWindow",
                                 MakeCallback (&CwndChange, stream, flowId));
}

int main (int argc, char *argv[])
{
  std::string tcpType1 = "ns3::TcpNewReno";
  std::string tcpType2 = "ns3::TcpCubic";
  std::string bottleneckBandwidth = "5Mbps";
  std::string bottleneckDelay = "20ms";
  std::string accessBandwidth = "100Mbps";
  std::string accessDelay = "2ms";
  double errorRate = 0.0001; // Introduced packet error rate
  uint32_t queueSizePackets = 50;
  double simulationTime = 20.0; // Seconds

  CommandLine cmd (__FILE__);
  cmd.AddValue ("tcpType1", "TCP variant for Flow 1", tcpType1);
  cmd.AddValue ("tcpType2", "TCP variant for Flow 2", tcpType2);
  cmd.AddValue ("bottleneckBandwidth", "Bottleneck link bandwidth", bottleneckBandwidth);
  cmd.AddValue ("errorRate", "Packet error rate on bottleneck", errorRate);
  cmd.Parse (argc, argv);

  // 1. Create Nodes (Dumbbell Topology)
  // S1---             ---R1
  //      \           /
  //       n0-------n1
  //      /           \
  // S2---             ---R2
  NodeContainer senders, receivers, routers;
  senders.Create (2);
  receivers.Create (2);
  routers.Create (2);

  // 2. Configure Channels / Links
  PointToPointHelper accessLink;
  accessLink.SetDeviceAttribute ("DataRate", StringValue (accessBandwidth));
  accessLink.SetChannelAttribute ("Delay", StringValue (accessDelay));

  PointToPointHelper bottleneckLink;
  bottleneckLink.SetDeviceAttribute ("DataRate", StringValue (bottleneckBandwidth));
  bottleneckLink.SetChannelAttribute ("Delay", StringValue (bottleneckDelay));
  bottleneckLink.SetQueue ("ns3::DropTailQueue", "MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, queueSizePackets)));

  // Introduce Error Model on the bottleneck link
  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorRate", DoubleValue (errorRate));
  em->SetAttribute ("ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));

  // Connect Senders and Receivers to Routers
  NetDeviceContainer dS0nR0 = accessLink.Install (senders.Get (0), routers.Get (0));
  NetDeviceContainer dS1nR0 = accessLink.Install (senders.Get (1), routers.Get (0));
  NetDeviceContainer dR0nR1 = bottleneckLink.Install (routers.Get (0), routers.Get (1));
  dR0nR1.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em)); // Set error model

  NetDeviceContainer dR1nR0 = accessLink.Install (routers.Get (1), receivers.Get (0));
  NetDeviceContainer dR1nR1 = accessLink.Install (routers.Get (1), receivers.Get (1));

  // 3. Install Internet Stack
  InternetStackHelper stack;
  stack.Install (senders);
  stack.Install (receivers);
  stack.Install (routers);

  // 4. Assign IP Addresses
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iS0nR0 = address.Assign (dS0nR0);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iS1nR0 = address.Assign (dS1nR0);

  address.SetBase ("10.2.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iR0nR1 = address.Assign (dR0nR1);

  address.SetBase ("10.3.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iR1nR0 = address.Assign (dR1nR0);

  address.SetBase ("10.3.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iR1nR1 = address.Assign (dR1nR1);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // 5. Configure Flow 1 (NewReno) Applications
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue (tcpType1));
  uint16_t port1 = 9001;
  Address sinkAddress1 (InetSocketAddress (iR1nR0.GetAddress (0), port1));
  PacketSinkHelper packetSinkHelper1 ("ns3::TcpSocketFactory", sinkAddress1);
  ApplicationContainer sinkApp1 = packetSinkHelper1.Install (receivers.Get (0));
  sinkApp1.Start (Seconds (0.0));
  sinkApp1.Stop (Seconds (simulationTime));

  BulkSendHelper sourceHelper1 ("ns3::TcpSocketFactory", sinkAddress1);
  sourceHelper1.SetAttribute ("MaxBytes", UintegerValue (0)); // Infinite send
  ApplicationContainer sourceApp1 = sourceHelper1.Install (senders.Get (0));
  sourceApp1.Start (Seconds (1.0));
  sourceApp1.Stop (Seconds (simulationTime));

  // 6. Configure Flow 2 (Cubic) Applications
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue (tcpType2));
  uint16_t port2 = 9002;
  Address sinkAddress2 (InetSocketAddress (iR1nR1.GetAddress (0), port2));
  PacketSinkHelper packetSinkHelper2 ("ns3::TcpSocketFactory", sinkAddress2);
  ApplicationContainer sinkApp2 = packetSinkHelper2.Install (receivers.Get (1));
  sinkApp2.Start (Seconds (0.0));
  sinkApp2.Stop (Seconds (simulationTime));

  BulkSendHelper sourceHelper2 ("ns3::TcpSocketFactory", sinkAddress2);
  sourceHelper2.SetAttribute ("MaxBytes", UintegerValue (0));
  ApplicationContainer sourceApp2 = sourceHelper2.Install (senders.Get (1));
  sourceApp2.Start (Seconds (1.0));
  sourceApp2.Stop (Seconds (simulationTime));

  // 7. Cwnd Tracing Setups
  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream1 = asciiTraceHelper.CreateFileStream ("flow1_cwnd.txt");
  Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream ("flow2_cwnd.txt");
  Simulator::Schedule (Seconds (1.001), &TraceCwnd, 1, stream1);
  Simulator::Schedule (Seconds (1.001), &TraceCwnd, 2, stream2);

  // 8. Flow Monitor for throughput, delay, and loss metrics
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  // 9. Run Simulation
  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();

  // 10. Extract Statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

  double t1 = 0, t2 = 0; // Throughputs for fairness calculation

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if (t.destinationPort == 9001 || t.destinationPort == 9002)
        {
          double throughput = i->second.rxBytes * 8.0 / (simulationTime - 1.0) / 1024 / 1024; // Mbps
          double avgDelay = i->second.delaySum.GetSeconds () / i->second.rxPackets;
          
          std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << "):\n";
          std::cout << "  Throughput: " << throughput << " Mbps\n";
          std::cout << "  Average Delay: " << avgDelay << " s\n";
          std::cout << "  Lost Packets: " << i->second.lostPackets << "\n";
          std::cout << "  Retransmitted Packets: " << i->second.packetsDropped.size() << "\n";

          if (t.destinationPort == 9001) t1 = throughput;
          if (t.destinationPort == 9002) t2 = throughput;
        }
    }

  // Calculate Jain's Fairness Index
  double jainsIndex = std::pow (t1 + t2, 2) / (2 * (std::pow (t1, 2) + std::pow (t2, 2)));
  std::cout << "\nJain's Fairness Index: " << jainsIndex << "\n";

  Simulator::Destroy ();
  return 0;
}
