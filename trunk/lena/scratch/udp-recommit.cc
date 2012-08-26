/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// Network topology
//
//       n0    n1   n2   n3
//       |     |    |    |
//       =================
//              LAN
//
// - UDP flows from n0 to n1 and back
// - DropTail queues 
// - Tracing of queues and packet receptions to file "udp-echo.tr"

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("UdpEchoExample");
static void setNewVelocity (Ptr<Node> node, Address a);
NodeContainer n;

int 
main (int argc, char *argv[])
{
//
// Users may find it convenient to turn on explicit debugging
// for selected modules; the below lines suggest how to do this
//
#if 0
  LogComponentEnable ("UdpEchoExample", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_ALL);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_ALL);
#endif
//
// Allow the user to override any of the defaults and the above Bind() at
// run-time, via command-line arguments
//
LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  
  bool useV6 = false;
  Address serverAddress, serverAddress1;

  CommandLine cmd;
  cmd.AddValue ("useIpv6", "Use Ipv6", useV6);
  cmd.Parse (argc, argv);
//
// Explicitly create the nodes required by the topology (shown above).
//
  NS_LOG_INFO ("Create nodes.");
  n.Create (4);
NodeContainer ni;
ni.Create(2);

  NodeContainer n1,n2,n3,ni2,ni3;
  n1.Add(n.Get(0));
n1.Add(n.Get(1));
n2.Add(n.Get(1));
n2.Add(n.Get(2));
n3.Add(n.Get(2));
n3.Add(n.Get(3));
ni2.Add(ni.Get(0));
ni2.Add(ni.Get(1));
ni3.Add(ni.Get(1));
ni3.Add(n.Get(3));

  InternetStackHelper internet;
  internet.Install (n);
  internet.Install (ni);

  NS_LOG_INFO ("Create channels.");
//
// Explicitly create the channels required by the topology (shown above).
//
  PointToPointHelper csma11,csma12,csma13,csma22,csma23;
  csma11.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  csma11.SetChannelAttribute ("Delay", StringValue ("2ms"));
  csma12.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  csma12.SetChannelAttribute ("Delay", StringValue ("2ms"));
  csma13.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  csma13.SetChannelAttribute ("Delay", StringValue ("2ms"));
  csma22.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  csma22.SetChannelAttribute ("Delay", StringValue ("2ms"));
  csma23.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  csma23.SetChannelAttribute ("Delay", StringValue ("2ms"));
  //csma.SetDeviceAttribute ("Mtu", UintegerValue (1400));
  NetDeviceContainer d11 = csma11.Install (n1);
NetDeviceContainer d12 = csma12.Install (n2);
NetDeviceContainer d13 = csma13.Install (n3);
NetDeviceContainer d22 = csma22.Install (ni2);
NetDeviceContainer d23 = csma23.Install (ni3);


//
// We've got the "hardware" in place.  Now we need to add IP addresses.
//
  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  NS_LOG_INFO ("Assign IP Addresses.");
  if (useV6 == false)
    {
      Ipv4AddressHelper ipv4;
      ipv4.SetBase ("10.1.1.0", "255.255.255.0");
      Ipv4InterfaceContainer i11 = ipv4.Assign (d11);
      ipv4.SetBase ("10.1.2.0", "255.255.255.0");
      Ipv4InterfaceContainer i12 = ipv4.Assign (d12);
      Ipv4InterfaceContainer i13 = ipv4.Assign (d13);
      Ptr<Ipv4> ipv4p = n.Get(2)->GetObject<Ipv4> ();
      Ptr<Ipv4StaticRouting> sr = ipv4RoutingHelper.GetStaticRouting (ipv4p);
      sr->AddNetworkRouteTo (i11.GetAddress(0), Ipv4Mask ("255.255.255.0"), 1);
      ipv4p = n.Get(0)->GetObject<Ipv4> ();
      sr = ipv4RoutingHelper.GetStaticRouting (ipv4p);
      sr->AddNetworkRouteTo (i12.GetAddress(1), Ipv4Mask ("255.255.255.0"), 1);
      sr->AddNetworkRouteTo (i13.GetAddress(1), Ipv4Mask ("255.255.255.0"), 1);
      ipv4.SetBase ("10.1.3.0", "255.255.255.0");
      ipv4p = n.Get(3)->GetObject<Ipv4> ();
      sr = ipv4RoutingHelper.GetStaticRouting (ipv4p);
      sr->AddNetworkRouteTo (i11.GetAddress(0), Ipv4Mask ("255.255.255.0"), 1);
      serverAddress = Address(i11.GetAddress (0));
      Ipv4AddressHelper ipv41;
      ipv41.SetBase ("11.2.2.0", "255.255.255.0");
      Ipv4InterfaceContainer i22 = ipv41.Assign (d22);
      ipv41.SetBase ("11.2.3.0", "255.255.255.0");
      Ipv4InterfaceContainer i23 = ipv41.Assign (d23);
      serverAddress1 = Address(i22.GetAddress (0));
    }
  else
    {
      Ipv6AddressHelper ipv6;
      ipv6.NewNetwork ("2001:0000:f00d:cafe::", 64);
      Ipv6InterfaceContainer i6 = ipv6.Assign (d11);
      serverAddress = Address(i6.GetAddress (1,1));
    }

  NS_LOG_INFO ("Create Applications.");
//
// Create a UdpEchoServer application on node one.
//
  uint16_t port = 9;  // well-known echo port number
  UdpEchoServerHelper server (port);
  ApplicationContainer apps = server.Install (n.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));
UdpEchoServerHelper server1 (port);
  ApplicationContainer apps1 = server.Install (ni.Get (0));
  apps1.Start (Seconds (1.0));
  apps1.Stop (Seconds (10.0));

//
// Create a UdpEchoClient application to send UDP datagrams from node zero to
// node one.
//
  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 10;
  Time interPacketInterval = Seconds (1.);
  UdpEchoClientHelper client (serverAddress, port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client.SetAttribute ("PacketSize", UintegerValue (packetSize));
  apps = client.Install (n.Get (3));
  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (10.0));

#if 0
//
// Users may find it convenient to initialize echo packets with actual data;
// the below lines suggest how to do this
//
  client.SetFill (apps.Get (0), "Hello World");

  client.SetFill (apps.Get (0), 0xa5, 1024);

  uint8_t fill[] = { 0, 1, 2, 3, 4, 5, 6};
  client.SetFill (apps.Get (0), fill, sizeof(fill), 1024);
#endif

  //AsciiTraceHelper ascii;
  //csma.EnableAsciiAll (ascii.CreateFileStream ("udp-echo.tr"));
  csma11.EnablePcapAll ("udp-echo11", false);

  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
//
// Now, do the actual simulation.
//
  Simulator::Schedule (Seconds (5), &setNewVelocity, n.Get(3), serverAddress1);
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}

static void setNewVelocity (Ptr<Node> node, Address a)
{
  Ptr<Application> aplic=node ->GetApplication(0);
  Application* apl=PeekPointer(aplic);
  UdpEchoClient* udpc;
  udpc = (UdpEchoClient*) apl;
  udpc -> StopApplication();
  std::cout << Simulator::Now().GetSeconds () << ": Address Changed "<<std::endl;
  udpc -> SetRemote(a,9);
  std::cout << Simulator::Now().GetSeconds () << ": Address Changed "<<std::endl;
  udpc -> StartApplication();
  std::cout << Simulator::Now().GetSeconds () << ": Address Changed "<<std::endl;
}
