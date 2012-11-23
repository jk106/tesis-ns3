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
#include "ns3/lte-module.h"
#include "ns3/netchart.h"
#include "ns3/network-manager.h"
#include "ns3/flow-monitor-helper.h"
#include <iostream>

NS_LOG_COMPONENT_DEFINE ("WimaxSimpleExample");

using namespace ns3;
//static void setNewDevice (Ptr<Node> node,uint8_t index);

int main (int argc, char *argv[])
{
  Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));
  bool verbose = false;

  int duration = 2000.2, schedType = 0;
  int numberOfNodes=7;
  WimaxHelper::SchedulerType scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;

  CommandLine cmd;
  cmd.AddValue ("scheduler", "type of scheduler to use with the network devices", schedType);
  cmd.AddValue ("duration", "duration of the simulation in seconds", duration);
  cmd.AddValue ("verbose", "turn on all WimaxNetDevice log components", verbose);
  cmd.Parse (argc, argv);
  //LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  LogComponentEnable ("MihNetDevice", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
  //LogComponentEnable ("WifiNetDevice", LOG_LEVEL_ALL);
  //LogComponentEnable ("Node", LOG_LEVEL_ALL);
  //LogComponentEnable ("Ipv4StaticRouting", LOG_LEVEL_ALL);
  //LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);
  //LogComponentEnable ("ArpL3Protocol", LOG_LEVEL_ALL);
  //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

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

  std::vector<Ptr<Node> > nodesWifi1,nodesWimax,nodesLte,nodesWifi2;
  std::vector<int > indexWifi1_down,indexWimax_down,indexLte_down,indexWifi2_down;
  NetChart *netWifi1= new NetChart();
  NetChart *netWifi2= new NetChart();
  NetChart *netWimax= new NetChart();
  NetChart *netLte= new NetChart();
  NodeContainer ssNodes;
  NodeContainer bsNodes;//WiFi and WiMAX
  NodeContainer bsNodes1;//LTE
  Ptr<LteHelper> lteHelper;	//Define LTE	
  Ptr<EpcHelper>  epcHelper;	//Define EPC
  ssNodes.Create (numberOfNodes+1);
  bsNodes.Create (1);
  bsNodes1.Create(1);
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
  nodesWifi1.push_back(p2pNodes.Get(0));
  indexWifi1_down.push_back(2);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  NodeContainer wifiApNodes;
  wifiApNodes.Add(p2pNodes.Get (0));
  wifiApNodes.Create(1);
  nodesWifi2.push_back(wifiApNodes.Get(1));
  indexWifi2_down.push_back(2);


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
  NodeContainer n;
  for(int i=0;i<numberOfNodes;i++)
  {
    n.Add(ssNodes.Get(i));
  }
  staDevices = wifi.Install (phy, mac, n);
  
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNodes);
  apDevices.Get(0)->SetAddress(Mac48Address("AA:BB:CC:DD:EE:FF"));
  apDevices.Get(1)->SetAddress(Mac48Address("AA:BB:CC:DD:EE:FF"));

  MobilityHelper mobility1;

  mobility1.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));




  mobility1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility1.Install (wifiApNodes);
(wifiApNodes.Get(0) -> GetObject<ConstantPositionMobilityModel>()) -> SetPosition(Vector(00.0, 0.0, 0.0));
(wifiApNodes.Get(1) -> GetObject<ConstantPositionMobilityModel>()) -> SetPosition(Vector(1000.0, 0.0, 0.0));
  netWifi1->SetApLocation(Vector(00.0, 0.0, 0.0));
  netWifi2->SetApLocation(Vector(1000.0, 0.0, 0.0));
  stack1.Install (csmaNodes);
  stack1.Install (wifiApNodes);
  
  Ipv4AddressHelper address1;

  address1.SetBase ("11.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address1.Assign (p2pDevices);

  address1.SetBase ("11.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address1.Assign (csmaDevices);  
  



  WimaxHelper wimax;

  NetDeviceContainer ssDevs, bsDevs;

  ssDevs = wimax.Install (ssNodes,
                          WimaxHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                          WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
                          scheduler);
  bsDevs = wimax.Install (bsNodes, WimaxHelper::DEVICE_TYPE_BASE_STATION, WimaxHelper::SIMPLE_PHY_TYPE_OFDM, scheduler);

  nodesWimax.push_back(bsNodes.Get(0)); 

  //wimax.EnableAscii ("bs-devices", bsDevs);
  //wimax.EnableAscii ("ss-devices", ssDevs);

  Ptr<SubscriberStationNetDevice> ss[numberOfNodes+1];

  for (int i = 0; i < numberOfNodes+1; i++)
    {
      ss[i] = ssDevs.Get (i)->GetObject<SubscriberStationNetDevice> ();
      ss[i]->SetModulationType (WimaxPhy::MODULATION_TYPE_QAM16_12);
    }

  Ptr<BaseStationNetDevice> bs;

  bs = bsDevs.Get (0)->GetObject<BaseStationNetDevice> ();


  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc;
  positionAlloc = CreateObject<ListPositionAllocator> ();
  
  positionAlloc->Add (Vector (00.0, 0.0, 0.0)); //STA
  
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");  
  mobility.Install(n);
  
  
  Ptr<ConstantVelocityMobilityModel> cvm = ssNodes.Get(0)->GetObject<ConstantVelocityMobilityModel>();
  cvm->SetVelocity(Vector (1, 0, 0)); //move to left to right 10.0m/s
  cvm = ssNodes.Get(2)->GetObject<ConstantVelocityMobilityModel>();
  cvm->SetVelocity(Vector (1.3, 0, 0)); //move to left to right 10.0m/s
  cvm = ssNodes.Get(4)->GetObject<ConstantVelocityMobilityModel>();
  cvm->SetVelocity(Vector (5, 0, 0));
  cvm = ssNodes.Get(1)->GetObject<ConstantVelocityMobilityModel>();
  cvm->SetVelocity(Vector (-1, 0, 0));
  cvm = ssNodes.Get(3)->GetObject<ConstantVelocityMobilityModel>();
  cvm->SetVelocity(Vector (-1.2, 0, 0));
  cvm = ssNodes.Get(5)->GetObject<ConstantVelocityMobilityModel>();
  cvm->SetPosition(Vector (980, 0, 0));
  cvm = ssNodes.Get(6)->GetObject<ConstantVelocityMobilityModel>();
  cvm->SetPosition(Vector (1020, 0, 0));
  //cvm->SetPosition(Vector (00, 0, 0));

  positionAlloc = CreateObject<ListPositionAllocator> ();
  
  positionAlloc->Add (Vector (500.0, 40.0, 0.0)); //MAG1AP
  netWimax->SetApLocation(Vector(500.0, 40.0, 0.0));
  positionAlloc->Add (Vector (500.0, 40.0, 0.0));  //MAG2AP
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));  //MAG2AP
  netLte->SetApLocation(Vector(00.0, 0.0, 0.0));
  
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  
  mobility.Install (NodeContainer(bsNodes.Get(0),ssNodes.Get(numberOfNodes),bsNodes1.Get(0)));

  InternetStackHelper stack;
  stack.Install (bsNodes);
  
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2p.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2p.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010))); 
  NetDeviceContainer p2pDevices3;
  p2pDevices3=p2p.Install(NodeContainer(bsNodes.Get(0),p2pNodes.Get(1)));
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("11.1.4.0", "255.255.255.0");
  ipv4.Assign (p2pDevices3);
  indexWimax_down.push_back(2);
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  lteHelper = CreateObject<LteHelper> ();
  epcHelper = CreateObject<EpcHelper> ();

  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetSchedulerType("ns3::RrFfMacScheduler");
  lteHelper->SetAttribute ("PathlossModel",
                               StringValue ("ns3::FriisPropagationLossModel"));
 
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  nodesLte.push_back(pgw);
  indexLte_down.push_back(1);

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010))); 
  NetDeviceContainer p2pDevices2;
  p2pDevices2=p2ph.Install(NodeContainer(pgw,p2pNodes.Get(1)));

  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("11.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (p2pDevices2);
  
  NetDeviceContainer p2pDevices4;
  p2pDevices4=p2ph.Install(NodeContainer(wifiApNodes.Get(1),p2pNodes.Get(1)));

  ipv4h.SetBase ("11.1.7.0", "255.255.255.0");
  Ipv4InterfaceContainer wifi2IpIfaces = ipv4h.Assign (p2pDevices4);
  
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (csmaNodes.Get(nCsma)->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (csmaNodes.Get(0)->GetObject<Ipv4> ());
  //remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 3);

  NetDeviceContainer bsDevs1,ssDevs1;
  bsDevs1 = lteHelper->InstallEnbDevice (bsNodes1);
  
  ssDevs1=lteHelper->InstallUeDevice (ssNodes);
  for(int i=0;i<numberOfNodes+1;i++)
  { 
    lteHelper->Attach (ssDevs1.Get(i), bsDevs1.Get(0));   
  }
	    
  Ipv4InterfaceContainer iueIpIface;
  //iueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ssDevs1));
  
address1=epcHelper->GetAddressHelper();
  MihHelper help;
  ipv4h.SetBase ("11.1.5.0", "255.255.255.0");
  NetDeviceContainer mihdev=help.Install(n);
    
  Ipv4InterfaceContainer SSinterfaces = address1.Assign (NetDeviceContainer(mihdev,ssDevs.Get(numberOfNodes)));
  Ipv4InterfaceContainer APinterfaces = address1.Assign (apDevices);
ipv4h.SetBase ("11.1.6.0", "255.255.255.0");
  Ipv4InterfaceContainer BSinterface = ipv4h.Assign (bsDevs);
  lteHelper->ActivateEpsBearer (ssDevs1, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), EpcTft::Default (),SSinterfaces);
  for(int i=0;i<numberOfNodes;i++)
  {
    Ipv4Address ueAddr = SSinterfaces.GetAddress(i);
    std::cout<<"MIH adress: "<<ueAddr<<std::endl;
    help.SetAddress(ssNodes.Get(i),SSinterfaces.GetAddress(i));
    

  
  }
remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (ssNodes.Get(0)->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("11.1.2.0"), Ipv4Mask ("255.255.255.0"),1);
remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (ssNodes.Get(2)->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("11.1.2.0"), Ipv4Mask ("255.255.255.0"),1);
remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (ssNodes.Get(4)->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("11.1.2.0"), Ipv4Mask ("255.255.255.0"),1);
remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (ssNodes.Get(1)->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("11.1.2.0"), Ipv4Mask ("255.255.255.0"),1);
remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (ssNodes.Get(3)->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("11.1.2.0"), Ipv4Mask ("255.255.255.0"),1);
remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (ssNodes.Get(6)->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("11.1.2.0"), Ipv4Mask ("255.255.255.0"),1);
remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (ssNodes.Get(5)->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("11.1.2.0"), Ipv4Mask ("255.255.255.0"),1);
remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (p2pNodes.Get(0)->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("11.1.2.0"), Ipv4Mask ("255.255.255.0"),1);
remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (wifiApNodes.Get(1)->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("11.1.2.0"), Ipv4Mask ("255.255.255.0"),1);

  
  

  for(int i=0;i<5;i++)
  {
    help.Activate(ssNodes.Get(i),1,false);
    i++;
  }
  for(int i=1;i<5;i++)
  {
    help.Activate(ssNodes.Get(i),1,true);
    i++;
  }
  for(int i=5;i<numberOfNodes;i++)
  {
    help.Activate(ssNodes.Get(i),1,true);
  }
  
  for(int i=0;i<numberOfNodes+1;i++)
  {
    remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (bsNodes.Get(0)->GetObject<Ipv4> ());
    remoteHostStaticRouting->AddNetworkRouteTo (SSinterfaces.GetAddress(i), Ipv4Mask ("255.255.255.0"), 2);
  }
  remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (pgw->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("11.1.2.0"), Ipv4Mask ("255.255.255.0"),Ipv4Address ("11.1.3.0"), 2);
   
  Ipv4GlobalRoutingHelper g;
  Ptr<OutputStreamWrapper> routingStream= Create<OutputStreamWrapper>("dynamic10.routes",std::ios::out);
  g.PrintRoutingTableAllAt(Seconds(0),routingStream);
  routingStream= Create<OutputStreamWrapper>("dynamic942.routes",std::ios::out);
  g.PrintRoutingTableAllAt(Seconds(942),routingStream);
  routingStream= Create<OutputStreamWrapper>("dynamic941.routes",std::ios::out);
  g.PrintRoutingTableAllAt(Seconds(941),routingStream);	  
  
  
  if (verbose)
    {
      wimax.EnableLogComponents ();  // Turn on all wimax logging
    }
/*------------------------------*/

  netWifi1->SetId(1);
  netWifi1->SetTechnology(1);
  netWifi1->SetNodes(nodesWifi1,indexWifi1_down);
  netWifi2->SetId(2);
  netWifi2->SetTechnology(1);
  netWifi2->SetNodes(nodesWifi2,indexWifi2_down);
  netWimax->SetId(1);
  netWimax->SetTechnology(2);
  netWimax->SetNodes(nodesWimax,indexWimax_down);
  netLte->SetId(1);
  netLte->SetTechnology(3);
  netLte->SetNodes(nodesLte,indexLte_down);
  NetworkManager *netman=new NetworkManager();
  netman->AddNetChart(netWifi1,1);
  netman->AddNetChart(netWifi2,5);
  netman->AddNetChart(netWimax,3);
  netman->AddNetChart(netLte,4);
  netman->SetLma(p2pNodes.Get(1));
  for(int i=0;i<5;i++)
  {
    help.SetNetworkManager(ssNodes.Get(i),netman);
    help.SetNetId(ssNodes.Get(i),1);
  }
  for(int i=5;i<numberOfNodes;i++)
  {
    help.SetNetworkManager(ssNodes.Get(i),netman);
    help.SetNetId(ssNodes.Get(i),2);
  }
  help.SetQoS(ssNodes.Get(0),2);
  help.SetQoS(ssNodes.Get(2),2);

  /*------------------------------*/


UdpServerHelper udpServer99;
  ApplicationContainer serverApps99;
  UdpClientHelper udpClient99;
  ApplicationContainer clientApps99;

  udpServer99 = UdpServerHelper (100);

  serverApps99 = udpServer99.Install (ssNodes.Get (1));
  serverApps99.Start (Seconds (6));
  serverApps99.Stop (Seconds (duration));

  udpClient99 = UdpClientHelper (SSinterfaces.GetAddress (1), 100);
  udpClient99.SetAttribute ("MaxPackets", UintegerValue (60000));
  udpClient99.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
  udpClient99.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps99 = udpClient99.Install (csmaNodes.Get(0));
  clientApps99.Start (Seconds (6));
  clientApps99.Stop (Seconds (duration));

  UdpServerHelper udpServer98;
  ApplicationContainer serverApps98;
  UdpClientHelper udpClient98;
  ApplicationContainer clientApps98;

  udpServer98 = UdpServerHelper (100);

  serverApps98 = udpServer98.Install (csmaNodes.Get(nCsma));
  serverApps98.Start (Seconds (6));
  serverApps98.Stop (Seconds (duration));

  udpClient98 = UdpClientHelper (csmaInterfaces.GetAddress(nCsma), 100);
  udpClient98.SetAttribute ("MaxPackets", UintegerValue (600000));
  udpClient98.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
  udpClient98.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps98 = udpClient98.Install (ssNodes.Get(1));
  clientApps98.Start (Seconds (6));
  clientApps98.Stop (Seconds (duration));

UdpServerHelper udpServer97;
  ApplicationContainer serverApps97;
  UdpClientHelper udpClient97;
  ApplicationContainer clientApps97;

  udpServer97 = UdpServerHelper (100);

  serverApps97 = udpServer97.Install (ssNodes.Get (3));
  serverApps97.Start (Seconds (6));
  serverApps97.Stop (Seconds (duration));

  udpClient97 = UdpClientHelper (SSinterfaces.GetAddress (3), 100);
  udpClient97.SetAttribute ("MaxPackets", UintegerValue (60000));
  udpClient97.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
  udpClient97.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps97 = udpClient97.Install (csmaNodes.Get(0));
  clientApps97.Start (Seconds (6));
  clientApps97.Stop (Seconds (duration));

  UdpServerHelper udpServer96;
  ApplicationContainer serverApps96;
  UdpClientHelper udpClient96;
  ApplicationContainer clientApps96;

  udpServer96 = UdpServerHelper (100);

  serverApps96 = udpServer96.Install (csmaNodes.Get(nCsma));
  serverApps96.Start (Seconds (6));
  serverApps96.Stop (Seconds (duration));

  udpClient96 = UdpClientHelper (csmaInterfaces.GetAddress(nCsma), 100);
  udpClient96.SetAttribute ("MaxPackets", UintegerValue (600000));
  udpClient96.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
  udpClient96.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps96 = udpClient96.Install (ssNodes.Get(3));
  clientApps96.Start (Seconds (6));
  clientApps96.Stop (Seconds (duration));

UdpServerHelper udpServer110;
  ApplicationContainer serverApps110;
  UdpClientHelper udpClient110;
  ApplicationContainer clientApps110;

  udpServer110 = UdpServerHelper (100);

  serverApps110 = udpServer110.Install (ssNodes.Get (5));
  serverApps110.Start (Seconds (6));
  serverApps110.Stop (Seconds (duration));

  udpClient110 = UdpClientHelper (SSinterfaces.GetAddress (5), 100);
  udpClient110.SetAttribute ("MaxPackets", UintegerValue (60000));
  udpClient110.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
  udpClient110.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps110 = udpClient110.Install (csmaNodes.Get(0));
  clientApps110.Start (Seconds (6));
  clientApps110.Stop (Seconds (duration));

  UdpServerHelper udpServer109;
  ApplicationContainer serverApps109;
  UdpClientHelper udpClient109;
  ApplicationContainer clientApps109;

  udpServer109 = UdpServerHelper (100);

  serverApps109 = udpServer109.Install (csmaNodes.Get(nCsma));
  serverApps109.Start (Seconds (6));
  serverApps109.Stop (Seconds (duration));

  udpClient109 = UdpClientHelper (csmaInterfaces.GetAddress(nCsma), 100);
  udpClient109.SetAttribute ("MaxPackets", UintegerValue (600000));
  udpClient109.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
  udpClient109.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps109 = udpClient109.Install (ssNodes.Get(5));
  clientApps109.Start (Seconds (6));
  clientApps109.Stop (Seconds (duration));

UdpServerHelper udpServer108;
  ApplicationContainer serverApps108;
  UdpClientHelper udpClient108;
  ApplicationContainer clientApps108;

  udpServer108 = UdpServerHelper (100);

  serverApps108 = udpServer108.Install (ssNodes.Get (6));
  serverApps108.Start (Seconds (6));
  serverApps108.Stop (Seconds (duration));

  udpClient108 = UdpClientHelper (SSinterfaces.GetAddress (6), 100);
  udpClient108.SetAttribute ("MaxPackets", UintegerValue (60000));
  udpClient108.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
  udpClient108.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps108 = udpClient108.Install (csmaNodes.Get(0));
  clientApps108.Start (Seconds (6));
  clientApps108.Stop (Seconds (duration));

  UdpServerHelper udpServer107;
  ApplicationContainer serverApps107;
  UdpClientHelper udpClient107;
  ApplicationContainer clientApps107;

  udpServer107 = UdpServerHelper (100);

  serverApps107 = udpServer107.Install (csmaNodes.Get(nCsma));
  serverApps107.Start (Seconds (6));
  serverApps107.Stop (Seconds (duration));

  udpClient107 = UdpClientHelper (csmaInterfaces.GetAddress(nCsma), 100);
  udpClient107.SetAttribute ("MaxPackets", UintegerValue (600000));
  udpClient107.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
  udpClient107.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps107 = udpClient107.Install (ssNodes.Get(6));
  clientApps107.Start (Seconds (6));
  clientApps107.Stop (Seconds (duration));





/*----------------------------------------*/

UdpServerHelper udpServer95;
  ApplicationContainer serverApps95;
  UdpClientHelper udpClient95;
  ApplicationContainer clientApps95;

  udpServer95 = UdpServerHelper (100);

  serverApps95 = udpServer95.Install (ssNodes.Get (2));
  serverApps95.Start (Seconds (6));
  serverApps95.Stop (Seconds (duration));

  udpClient95 = UdpClientHelper (SSinterfaces.GetAddress (2), 100);
  udpClient95.SetAttribute ("MaxPackets", UintegerValue (60000));
  udpClient95.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
  udpClient95.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps95 = udpClient95.Install (csmaNodes.Get(0));
  clientApps95.Start (Seconds (6));
  clientApps95.Stop (Seconds (duration));

  UdpServerHelper udpServer94;
  ApplicationContainer serverApps94;
  UdpClientHelper udpClient94;
  ApplicationContainer clientApps94;

  udpServer94 = UdpServerHelper (100);

  serverApps94 = udpServer94.Install (csmaNodes.Get(nCsma));
  serverApps94.Start (Seconds (6));
  serverApps94.Stop (Seconds (duration));

  udpClient94 = UdpClientHelper (csmaInterfaces.GetAddress(nCsma), 100);
  udpClient94.SetAttribute ("MaxPackets", UintegerValue (600000));
  udpClient94.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
  udpClient94.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps94 = udpClient94.Install (ssNodes.Get(2));
  clientApps94.Start (Seconds (6));
  clientApps94.Stop (Seconds (duration));

UdpServerHelper udpServer93;
  ApplicationContainer serverApps93;
  UdpClientHelper udpClient93;
  ApplicationContainer clientApps93;

  udpServer93 = UdpServerHelper (100);

  serverApps93 = udpServer93.Install (ssNodes.Get (4));
  serverApps93.Start (Seconds (6));
  serverApps93.Stop (Seconds (duration));

  udpClient93 = UdpClientHelper (SSinterfaces.GetAddress (4), 100);
  udpClient93.SetAttribute ("MaxPackets", UintegerValue (60000));
  udpClient93.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
  udpClient93.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps93 = udpClient93.Install (csmaNodes.Get(0));
  clientApps93.Start (Seconds (6));
  clientApps93.Stop (Seconds (duration));

  UdpServerHelper udpServer92;
  ApplicationContainer serverApps92;
  UdpClientHelper udpClient92;
  ApplicationContainer clientApps92;

  udpServer92 = UdpServerHelper (100);

  serverApps92 = udpServer92.Install (csmaNodes.Get(nCsma));
  serverApps92.Start (Seconds (6));
  serverApps92.Stop (Seconds (duration));

  udpClient92 = UdpClientHelper (csmaInterfaces.GetAddress(nCsma), 100);
  udpClient92.SetAttribute ("MaxPackets", UintegerValue (600000));
  udpClient92.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
  udpClient92.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps92 = udpClient92.Install (ssNodes.Get(4));
  clientApps92.Start (Seconds (6));
  clientApps92.Stop (Seconds (duration));


/*------------------------------------*/

  UdpServerHelper udpServer;
  ApplicationContainer serverApps;
  UdpClientHelper udpClient;
  ApplicationContainer clientApps;

  udpServer = UdpServerHelper (100);

  serverApps = udpServer.Install (ssNodes.Get (0));
  serverApps.Start (Seconds (6));
  serverApps.Stop (Seconds (duration));

  udpClient = UdpClientHelper (SSinterfaces.GetAddress (0), 100);
  udpClient.SetAttribute ("MaxPackets", UintegerValue (60000));
  udpClient.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
  udpClient.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps = udpClient.Install (csmaNodes.Get(0));
  clientApps.Start (Seconds (6));
  clientApps.Stop (Seconds (duration));

  UdpServerHelper udpServer3;
  ApplicationContainer serverApps3;
  UdpClientHelper udpClient3;
  ApplicationContainer clientApps3;

  udpServer3 = UdpServerHelper (100);

  serverApps3 = udpServer3.Install (csmaNodes.Get(0));
  serverApps3.Start (Seconds (6));
  serverApps3.Stop (Seconds (duration));

  udpClient3 = UdpClientHelper (csmaInterfaces.GetAddress(0), 100);
  udpClient3.SetAttribute ("MaxPackets", UintegerValue (600000));
  udpClient3.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
  udpClient3.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps3 = udpClient3.Install (ssNodes.Get(0));
  clientApps3.Start (Seconds (6));
  clientApps3.Stop (Seconds (duration));


  UdpServerHelper udpServer2;
  ApplicationContainer serverApps2;
  UdpClientHelper udpClient2;
  ApplicationContainer clientApps2;

  udpServer2 = UdpServerHelper (103);

  serverApps2= udpServer2.Install (ssNodes.Get (0));
  serverApps2.Start (Seconds (1));
  serverApps2.Stop (Seconds (duration));

  udpClient2 = UdpClientHelper (SSinterfaces.GetAddress (0), 103);
  udpClient2.SetAttribute ("MaxPackets", UintegerValue (6000));
  udpClient2.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  udpClient2.SetAttribute ("PacketSize", UintegerValue (12));

  clientApps2 = udpClient2.Install (pgw);
  clientApps2.Start (Seconds (1));
  clientApps2.Stop (Seconds (duration));

  UdpServerHelper udpServer4;
  ApplicationContainer serverApps4;
  UdpClientHelper udpClient4;
  ApplicationContainer clientApps4;

  udpServer4 = UdpServerHelper (100);

  serverApps4= udpServer4.Install (ssNodes.Get (0));
  serverApps4.Start (Seconds (1));
  serverApps4.Stop (Seconds (duration));

  udpClient4 = UdpClientHelper (SSinterfaces.GetAddress (0), 100);
  udpClient4.SetAttribute ("MaxPackets", UintegerValue (6000));
  udpClient4.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  udpClient4.SetAttribute ("PacketSize", UintegerValue (12));

  clientApps4 = udpClient4.Install (bsNodes.Get(0));
  clientApps4.Start (Seconds (1));
  clientApps4.Stop (Seconds (duration));

  

  UdpServerHelper udpServer5;
  ApplicationContainer serverApps5;
  UdpClientHelper udpClient5;
  ApplicationContainer clientApps5;

  udpServer5 = UdpServerHelper (103);

  serverApps5= udpServer5.Install (ssNodes.Get (2));
  serverApps5.Start (Seconds (1));
  serverApps5.Stop (Seconds (duration));

  udpClient5 = UdpClientHelper (SSinterfaces.GetAddress (2), 103);
  udpClient5.SetAttribute ("MaxPackets", UintegerValue (6000));
  udpClient5.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  udpClient5.SetAttribute ("PacketSize", UintegerValue (12));

  clientApps5 = udpClient5.Install (pgw);
  clientApps5.Start (Seconds (1));
  clientApps5.Stop (Seconds (duration));

  UdpServerHelper udpServer6;
  ApplicationContainer serverApps6;
  UdpClientHelper udpClient6;
  ApplicationContainer clientApps6;

  udpServer6 = UdpServerHelper (100);

  serverApps6= udpServer6.Install (ssNodes.Get (2));
  serverApps6.Start (Seconds (1));
  serverApps6.Stop (Seconds (duration));

  udpClient6 = UdpClientHelper (SSinterfaces.GetAddress (2), 100);
  udpClient6.SetAttribute ("MaxPackets", UintegerValue (6000));
  udpClient6.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  udpClient6.SetAttribute ("PacketSize", UintegerValue (12));

  clientApps6 = udpClient6.Install (bsNodes.Get(0));
  clientApps6.Start (Seconds (1));
  clientApps6.Stop (Seconds (duration));
  
  UdpServerHelper udpServer7;
  ApplicationContainer serverApps7;
  UdpClientHelper udpClient7;
  ApplicationContainer clientApps7;

  udpServer7 = UdpServerHelper (103);

  serverApps7= udpServer7.Install (ssNodes.Get (4));
  serverApps7.Start (Seconds (1));
  serverApps7.Stop (Seconds (duration));

  udpClient7 = UdpClientHelper (SSinterfaces.GetAddress (4), 103);
  udpClient7.SetAttribute ("MaxPackets", UintegerValue (6000));
  udpClient7.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  udpClient7.SetAttribute ("PacketSize", UintegerValue (12));

  clientApps7 = udpClient7.Install (pgw);
  clientApps7.Start (Seconds (1));
  clientApps7.Stop (Seconds (duration));

  UdpServerHelper udpServer8;
  ApplicationContainer serverApps8;
  UdpClientHelper udpClient8;
  ApplicationContainer clientApps8;

  udpServer8 = UdpServerHelper (100);

  serverApps8= udpServer8.Install (ssNodes.Get (4));
  serverApps8.Start (Seconds (1));
  serverApps8.Stop (Seconds (duration));

  udpClient8 = UdpClientHelper (SSinterfaces.GetAddress (4), 100);
  udpClient8.SetAttribute ("MaxPackets", UintegerValue (6000));
  udpClient8.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  udpClient8.SetAttribute ("PacketSize", UintegerValue (12));

  clientApps8 = udpClient8.Install (bsNodes.Get(0));
  clientApps8.Start (Seconds (1));
  clientApps8.Stop (Seconds (duration));

  UdpServerHelper udpServer9;
  ApplicationContainer serverApps9;
  UdpClientHelper udpClient9;
  ApplicationContainer clientApps9;

  udpServer9 = UdpServerHelper (103);

  serverApps9= udpServer9.Install (ssNodes.Get (2));
  serverApps9.Start (Seconds (1));
  serverApps9.Stop (Seconds (duration));

  udpClient9 = UdpClientHelper (SSinterfaces.GetAddress (2), 103);
  udpClient9.SetAttribute ("MaxPackets", UintegerValue (6000));
  udpClient9.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  udpClient9.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps9 = udpClient9.Install (wifiApNodes.Get(0));
  clientApps9.Start (Seconds (1));
  clientApps9.Stop (Seconds (duration));

  UdpServerHelper udpServer10;
  ApplicationContainer serverApps10;
  UdpClientHelper udpClient10;
  ApplicationContainer clientApps10;

  udpServer10 = UdpServerHelper (100);

  serverApps10= udpServer10.Install (ssNodes.Get (4));
  serverApps10.Start (Seconds (1));
  serverApps10.Stop (Seconds (duration));

  udpClient10 = UdpClientHelper (SSinterfaces.GetAddress (4), 100);
  udpClient10.SetAttribute ("MaxPackets", UintegerValue (6000));
  udpClient10.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  udpClient10.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps10 = udpClient10.Install (wifiApNodes.Get(0));
  clientApps10.Start (Seconds (1));
  clientApps10.Stop (Seconds (duration));

UdpServerHelper udpServer11;
  ApplicationContainer serverApps11;
  UdpClientHelper udpClient11;
  ApplicationContainer clientApps11;

  udpServer11 = UdpServerHelper (103);

  serverApps11= udpServer11.Install (ssNodes.Get (0));
  serverApps11.Start (Seconds (1));
  serverApps11.Stop (Seconds (duration));

  udpClient11 = UdpClientHelper (SSinterfaces.GetAddress (0), 103);
  udpClient11.SetAttribute ("MaxPackets", UintegerValue (6000));
  udpClient11.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  udpClient11.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps11 = udpClient11.Install (wifiApNodes.Get(0));
  clientApps11.Start (Seconds (1));
  clientApps11.Stop (Seconds (duration));

  UdpServerHelper udpServer12;
  ApplicationContainer serverApps12;
  UdpClientHelper udpClient12;
  ApplicationContainer clientApps12;

  udpServer12 = UdpServerHelper (100);

  serverApps12= udpServer12.Install (ssNodes.Get (0));
  serverApps12.Start (Seconds (1));
  serverApps12.Stop (Seconds (duration));

  udpClient12 = UdpClientHelper (SSinterfaces.GetAddress (0), 100);
  udpClient12.SetAttribute ("MaxPackets", UintegerValue (6000));
  udpClient12.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  udpClient12.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps12 = udpClient12.Install (wifiApNodes.Get(1));
  clientApps12.Start (Seconds (1));
  clientApps12.Stop (Seconds (duration));


  UdpServerHelper udpServer13;
  ApplicationContainer serverApps13;
  UdpClientHelper udpClient13;
  ApplicationContainer clientApps13;

  udpServer13 = UdpServerHelper (103);

  serverApps13= udpServer13.Install (ssNodes.Get (2));
  serverApps13.Start (Seconds (1));
  serverApps13.Stop (Seconds (duration));

  udpClient13 = UdpClientHelper (SSinterfaces.GetAddress (2), 103);
  udpClient13.SetAttribute ("MaxPackets", UintegerValue (6000));
  udpClient13.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  udpClient13.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps13 = udpClient13.Install (wifiApNodes.Get(1));
  clientApps13.Start (Seconds (1));
  clientApps13.Stop (Seconds (duration));

  UdpServerHelper udpServer14;
  ApplicationContainer serverApps14;
  UdpClientHelper udpClient14;
  ApplicationContainer clientApps14;

  udpServer14 = UdpServerHelper (100);

  serverApps14= udpServer14.Install (ssNodes.Get (4));
  serverApps14.Start (Seconds (1));
  serverApps14.Stop (Seconds (duration));

  udpClient14 = UdpClientHelper (SSinterfaces.GetAddress (4), 100);
  udpClient14.SetAttribute ("MaxPackets", UintegerValue (6000));
  udpClient14.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  udpClient14.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps14 = udpClient14.Install (wifiApNodes.Get(1));
  clientApps14.Start (Seconds (1));
  clientApps14.Stop (Seconds (duration));




  Simulator::Stop (Seconds (duration + 0.1));

  //wimax.EnablePcap ("wimax-simple-ss0", ssNodes.Get (0)->GetId (), ss[0]->GetIfIndex ());
  //wimax.EnablePcap ("wimax-simple-ss1", ssNodes.Get (1)->GetId (), ss[1]->GetIfIndex ());
  //wimax.EnablePcap ("wimax-simple-bs0", bsNodes.Get (0)->GetId (), bs->GetIfIndex ());
for(int i=0;i<numberOfNodes;i++)
{
  IpcsClassifierRecord DlClassifierUgs (Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        SSinterfaces.GetAddress (i),
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
  ss[i]->AddServiceFlow (DlServiceFlowUgs);
}
  IpcsClassifierRecord UlClassifierUgs (SSinterfaces.GetAddress (numberOfNodes),
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
  
  ss[numberOfNodes]->AddServiceFlow (UlServiceFlowUgs);

  

  NS_LOG_INFO ("Starting simulation.....");
  //Simulator::Schedule (Seconds (100), &setNewDevice, ssNodes.Get(0), 2);
  //Simulator::Schedule (Seconds (1000), &setNewDevice, ssNodes.Get(0), 3);

  FlowMonitorHelper fl;
  Ptr<FlowMonitor> monitor;
  monitor=fl.Install(ssNodes.Get(0));
  monitor=fl.Install(csmaNodes.Get(0));
  monitor->SetAttribute ("DelayBinWidth",
                           DoubleValue(0.01));
     monitor->SetAttribute ("JitterBinWidth",
                           DoubleValue(0.001));
     monitor->SetAttribute ("PacketSizeBinWidth",
                           DoubleValue(20));
Ptr<FlowMonitor> monitor1;
  monitor1=fl.Install(ssNodes.Get(1));
  monitor1=fl.Install(csmaNodes.Get(nCsma));
  monitor1->SetAttribute ("DelayBinWidth",
                           DoubleValue(0.01));
     monitor1->SetAttribute ("JitterBinWidth",
                           DoubleValue(0.001));
     monitor1->SetAttribute ("PacketSizeBinWidth",
                           DoubleValue(20));
Ptr<FlowMonitor> monitor2;
  monitor2=fl.Install(ssNodes.Get(2));
  monitor2=fl.Install(csmaNodes.Get(nCsma));
  monitor2->SetAttribute ("DelayBinWidth",
                           DoubleValue(0.01));
     monitor2->SetAttribute ("JitterBinWidth",
                           DoubleValue(0.001));
     monitor2->SetAttribute ("PacketSizeBinWidth",
                           DoubleValue(20));
Ptr<FlowMonitor> monitor3;
  monitor3=fl.Install(ssNodes.Get(3));
  monitor3=fl.Install(csmaNodes.Get(nCsma));
  monitor3->SetAttribute ("DelayBinWidth",
                           DoubleValue(0.01));
     monitor3->SetAttribute ("JitterBinWidth",
                           DoubleValue(0.001));
     monitor3->SetAttribute ("PacketSizeBinWidth",
                           DoubleValue(20));
Ptr<FlowMonitor> monitor4;
  monitor4=fl.Install(ssNodes.Get(4));
  monitor4=fl.Install(csmaNodes.Get(nCsma));
  monitor4->SetAttribute ("DelayBinWidth",
                           DoubleValue(0.01));
     monitor4->SetAttribute ("JitterBinWidth",
                           DoubleValue(0.001));
     monitor4->SetAttribute ("PacketSizeBinWidth",
                           DoubleValue(20));

Ptr<FlowMonitor> monitor5;
  monitor5=fl.Install(ssNodes.Get(5));
  monitor5=fl.Install(csmaNodes.Get(nCsma));
  monitor5->SetAttribute ("DelayBinWidth",
                           DoubleValue(0.01));
     monitor5->SetAttribute ("JitterBinWidth",
                           DoubleValue(0.001));
     monitor5->SetAttribute ("PacketSizeBinWidth",
                           DoubleValue(20));

Ptr<FlowMonitor> monitor6;
  monitor6=fl.Install(ssNodes.Get(6));
  monitor6=fl.Install(csmaNodes.Get(nCsma));
  monitor6->SetAttribute ("DelayBinWidth",
                           DoubleValue(0.01));
     monitor6->SetAttribute ("JitterBinWidth",
                           DoubleValue(0.001));
     monitor6->SetAttribute ("PacketSizeBinWidth",
                           DoubleValue(20));

  
  Simulator::Run ();
  monitor->SerializeToXmlFile("result10-0.xml",true,true);
  
  ss[0] = 0;
  ss[1] = 0;
  bs = 0;

  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}
