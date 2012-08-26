/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2007,2008, 2009 INRIA, UDcast
 *
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
 *
 * Author: Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
 *                              <amine.ismail@udcast.com>
 */

//
// Default network topology includes a base station (BS) and 2
// subscriber station (SS).

//      +-----+
//      | SS0 |
//      +-----+
//     10.1.1.1
//      -------
//        ((*))
//
//                  10.1.1.7
//               +------------+
//               |Base Station| ==((*))
//               +------------+
//
//        ((*))
//       -------
//      10.1.1.2
//       +-----+
//       | SS1 |
//       +-----+

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/csma-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wimax-module.h"
#include "ns3/internet-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/ipcs-classifier-record.h"
#include "ns3/service-flow.h"
#include "ns3/mih-helper.h"
#include "ns3/mih-net-device.h"
#include <iostream>

NS_LOG_COMPONENT_DEFINE ("WimaxSimpleExample");

using namespace ns3;
static void setNewDevice (Ptr<Node> node,uint8_t index);

int main (int argc, char *argv[])
{
  bool verbose = false;

  int duration = 20.2, schedType = 0;
  WimaxHelper::SchedulerType scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;

  CommandLine cmd;
  cmd.AddValue ("scheduler", "type of scheduler to use with the network devices", schedType);
  cmd.AddValue ("duration", "duration of the simulation in seconds", duration);
  cmd.AddValue ("verbose", "turn on all WimaxNetDevice log components", verbose);
  cmd.Parse (argc, argv);
  LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  switch (schedType)
    {
    case 0:
      scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;
      break;
    case 1:
      scheduler = WimaxHelper::SCHED_TYPE_MBQOS;
      break;
    case 2:
      scheduler = WimaxHelper::SCHED_TYPE_RTPS;
      break;
    default:
      scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;
    }

  NodeContainer ssNodes;
  NodeContainer bsNodes;

  ssNodes.Create (2);
  bsNodes.Create (1);
  InternetStackHelper stack1;
  stack1.Install (ssNodes);



uint32_t nCsma = 3;

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Create (nCsma);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  NodeContainer wifiApNode = p2pNodes.Get (0);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();

  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, ssNodes.Get(0));

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  MobilityHelper mobility1;

  mobility1.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));




  mobility1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility1.Install (wifiApNode);
(wifiApNode.Get(0) -> GetObject<ConstantPositionMobilityModel>()) -> SetPosition(Vector(00.0, 0.0, 0.0));
  stack1.Install (csmaNodes);
  stack1.Install (wifiApNode);
  
  Ipv4AddressHelper address1;

  address1.SetBase ("11.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address1.Assign (p2pDevices);

  address1.SetBase ("11.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address1.Assign (csmaDevices);


  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps1 = echoServer.Install (csmaNodes.Get (nCsma));
  serverApps1.Start (Seconds (1.0));
  serverApps1.Stop (Seconds (duration));

  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1000));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps1 = 
    echoClient.Install (ssNodes.Get (0));
  clientApps1.Start (Seconds (2.0));
  clientApps1.Stop (Seconds (duration));

  
  



  WimaxHelper wimax;

  NetDeviceContainer ssDevs, bsDevs;

  ssDevs = wimax.Install (ssNodes,
                          WimaxHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                          WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
                          scheduler);
  bsDevs = wimax.Install (bsNodes, WimaxHelper::DEVICE_TYPE_BASE_STATION, WimaxHelper::SIMPLE_PHY_TYPE_OFDM, scheduler);

  wimax.EnableAscii ("bs-devices", bsDevs);
  wimax.EnableAscii ("ss-devices", ssDevs);

  Ptr<SubscriberStationNetDevice> ss[2];

  for (int i = 0; i < 2; i++)
    {
      ss[i] = ssDevs.Get (i)->GetObject<SubscriberStationNetDevice> ();
      ss[i]->SetModulationType (WimaxPhy::MODULATION_TYPE_QAM16_12);
    }

  Ptr<BaseStationNetDevice> bs;

  bs = bsDevs.Get (0)->GetObject<BaseStationNetDevice> ();


  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc;
  positionAlloc = CreateObject<ListPositionAllocator> ();
  
  positionAlloc->Add (Vector (0.0, 70.0, 0.0)); //STA
  
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");  
  mobility.Install(ssNodes.Get(0));
  
  Ptr<ConstantVelocityMobilityModel> cvm = ssNodes.Get(0)->GetObject<ConstantVelocityMobilityModel>();
  cvm->SetVelocity(Vector (0, 0, 0)); //move to left to right 10.0m/s

  positionAlloc = CreateObject<ListPositionAllocator> ();
  
  positionAlloc->Add (Vector (-0.0, 40.0, 0.0)); //MAG1AP
  positionAlloc->Add (Vector (0.0, 40.0, 0.0));  //MAG2AP
  
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  
  mobility.Install (NodeContainer(bsNodes.Get(0),ssNodes.Get(1)));


  InternetStackHelper stack;
  stack.Install (bsNodes);
  
  
  MihHelper help;
  NetDeviceContainer mihdev=help.Install(ssNodes.Get(0));


  address1.SetBase ("11.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer APinterfaces = address1.Assign (apDevices);
  Ipv4InterfaceContainer SSinterfaces = address1.Assign (NetDeviceContainer(mihdev,ssDevs.Get(1)));
  Ipv4InterfaceContainer BSinterface = address1.Assign (bsDevs);
  help.SetAddress(ssNodes.Get(0),SSinterfaces.GetAddress(0));
  help.Activate(ssNodes.Get(0),1);
Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


  if (verbose)
    {
      wimax.EnableLogComponents ();  // Turn on all wimax logging
    }
  /*------------------------------*/
  UdpServerHelper udpServer;
  ApplicationContainer serverApps;
  UdpClientHelper udpClient;
  ApplicationContainer clientApps;

  udpServer = UdpServerHelper (100);

  serverApps = udpServer.Install (ssNodes.Get (0));
  serverApps.Start (Seconds (6));
  serverApps.Stop (Seconds (duration));

  udpClient = UdpClientHelper (SSinterfaces.GetAddress (0), 100);
  udpClient.SetAttribute ("MaxPackets", UintegerValue (1200));
  udpClient.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  udpClient.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps = udpClient.Install (ssNodes.Get (1));
  clientApps.Start (Seconds (6));
  clientApps.Stop (Seconds (duration));


UdpServerHelper udpServer2;
  ApplicationContainer serverApps2;
  UdpClientHelper udpClient2;
  ApplicationContainer clientApps2;

  udpServer2 = UdpServerHelper (100);

  serverApps2= udpServer2.Install (ssNodes.Get (1));
  serverApps2.Start (Seconds (2));
  serverApps2.Stop (Seconds (duration));

  udpClient2 = UdpClientHelper (SSinterfaces.GetAddress (1), 100);
  udpClient2.SetAttribute ("MaxPackets", UintegerValue (1200));
  udpClient2.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  udpClient2.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps2 = udpClient2.Install (ssNodes.Get (0));
  clientApps2.Start (Seconds (2));
  clientApps2.Stop (Seconds (duration));

  Simulator::Stop (Seconds (duration + 0.1));

  wimax.EnablePcap ("wimax-simple-ss0", ssNodes.Get (0)->GetId (), ss[0]->GetIfIndex ());
  wimax.EnablePcap ("wimax-simple-ss1", ssNodes.Get (1)->GetId (), ss[1]->GetIfIndex ());
  wimax.EnablePcap ("wimax-simple-bs0", bsNodes.Get (0)->GetId (), bs->GetIfIndex ());

  IpcsClassifierRecord DlClassifierUgs (Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        SSinterfaces.GetAddress (0),
                                        Ipv4Mask ("255.255.255.255"),
                                        0,
                                        65000,
                                        100,
                                        100,
                                        17,
                                        1);
  ServiceFlow DlServiceFlowUgs = wimax.CreateServiceFlow (ServiceFlow::SF_DIRECTION_DOWN,
                                                          ServiceFlow::SF_TYPE_RTPS,
                                                          DlClassifierUgs);

  IpcsClassifierRecord UlClassifierUgs (SSinterfaces.GetAddress (1),
                                        Ipv4Mask ("255.255.255.255"),
                                        Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        0,
                                        65000,
                                        100,
                                        100,
                                        17,
                                        1);
  ServiceFlow UlServiceFlowUgs = wimax.CreateServiceFlow (ServiceFlow::SF_DIRECTION_UP,
                                                          ServiceFlow::SF_TYPE_RTPS,
                                                          UlClassifierUgs);
  ss[0]->AddServiceFlow (DlServiceFlowUgs);
  ss[1]->AddServiceFlow (UlServiceFlowUgs);

  

  NS_LOG_INFO ("Starting simulation.....");
  Simulator::Schedule (Seconds (10), &setNewDevice, ssNodes.Get(0), 2);
  
  Simulator::Run ();

  ss[0] = 0;
  ss[1] = 0;
  bs = 0;

  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}

static void setNewDevice (Ptr<Node> node, uint8_t index)
{
  Ptr<NetDevice> aplic=node ->GetDevice(node->GetNDevices()-1);
  NetDevice* apl=PeekPointer(aplic);
  MihNetDevice* udpc;
  udpc = (MihNetDevice*) apl;
  udpc -> Activate(index);
  std::cout << Simulator::Now().GetSeconds () << ": Device Swapped "<<std::endl;
}
