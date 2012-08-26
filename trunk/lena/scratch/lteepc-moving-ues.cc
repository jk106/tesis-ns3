/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Longhao Zou(zoulonghao@gmail.com)
 * Date: 16/03/2012 
 * PEL@DCU
 */

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/flow-monitor-helper.h"
#include <iomanip>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
//#include "ns3/gtk-config-store.h"

using namespace ns3;
using std::vector;
/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeB,
 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
 * It also  starts yet another flow between each UE pair.
 */
NS_LOG_COMPONENT_DEFINE ("LteEpcSimFirst");





class LteEpcSim
{
public:
	  uint16_t numberOfeNodeBs;	//Default number of eNodeB
  	uint16_t numberOfUEs; 		//Default number of ues attached to each eNodeB
  	double simTime;
  	double enbD;
  	double enbH;
  	double ueD;
  	double velocity;
  	double distance;
  	double newVelocity;
  	double newDistance;
  	// Inter packet interval in ms
	  // double interPacketInterval;

	
	  Ptr<LteHelper> lteHelper;	//Define LTE	
	  Ptr<EpcHelper>  epcHelper;	//Define EPC

    NodeContainer remoteHostContainer;		//Define the Remote Host
  	NodeContainer enbNodes;					//Define the eNodeBs

	  vector <NodeContainer> ueNodes;		//Define a Vector to store all the UEs

	  NetDeviceContainer internetDevices; 	//Define the Network Devices in the Connection between EPC and the remote host
	  NetDeviceContainer enbLteDevs;			//Define the Network Devices in the eNodeBs

	  vector <NetDeviceContainer> ueLteDevs;	//Define a Vector to store all Network Devices of UEs

   	Ptr<Node> pgw;				//Define the Packet Data Network Gateway(P-GW)	
	  Ptr<Node> remoteHost;		//Define the node of remote Host
	
	  InternetStackHelper internet;			//Define the internet stack	
	  PointToPointHelper p2ph;				//Define Connection between EPC and the Remote Host
	  Ipv4AddressHelper ipv4h;				//Ipv4 address helper
	  Ipv4StaticRoutingHelper ipv4RoutingHelper;	//Ipv4 static routing helper	
	  Ptr<Ipv4StaticRouting> remoteHostStaticRouting;

	  Ipv4InterfaceContainer internetIpIfaces;	//Ipv4 interfaces
	
	  vector <Ipv4InterfaceContainer> ueIpIface;	//UE interfaces

	  //Ipv4Address remoteHostAddr;				//IP address of Remote Host	

	  //Configure Mobility
	  MobilityHelper mobility;
	  Ptr<ListPositionAllocator> positionAlloc;
	  vector<Vector> enbPosition;
	  Ptr<ConstantVelocityMobilityModel> VMM_0_0;	//Velocity Mobility Model of UE_0 attached to eNodeB_0
	  
    FlowMonitorHelper fl;
    Ptr<FlowMonitor> monitor;
    
public:
	  LteEpcSim();
	  ~LteEpcSim();
	
	  
	  void Configure(int argc, char** agrv);
	  void InstallEPC();
	  void InstallENBUEs();
	  void InstallMobility();
	  void InstallApplications();
	  void SetFlowMonitor();
	  int Run();
};


LteEpcSim::LteEpcSim()
{
	  numberOfeNodeBs = 2;//Default number of eNodeB
  	numberOfUEs = 2; //Default number of ues attached to each eNodeB
  	simTime = 100.0;
  	enbD = 500.0;
  	enbH = 10.0;
  	ueD= 10.0;
  	velocity = 750.0;
  	distance = 1000.0;
  	newVelocity = 750.0;
  	newDistance = 500.0;
  	// Inter packet interval in ms
	  // double interPacketInterval = 1;
  	// double interPacketInterval = 100;

}

LteEpcSim::~LteEpcSim()
{
}





void LteEpcSim::Configure(int argc, char** argv)
{
	  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
    GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
	  Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));

  	// Command line arguments
  	CommandLine cmd;
  	cmd.AddValue("numberOfeNodeBs", "Number of eNodeBs", numberOfeNodeBs);
  	cmd.AddValue("numberOfeNodeBs", "Number of UEs attach to each eNodeB", numberOfUEs);
  	cmd.AddValue("simTime", "Total duration of the simulation (in seconds)",simTime);
  	cmd.AddValue("enbD", "Distance between eNodeB and Center",enbD);
  	cmd.AddValue("enbH","Height of each eNodeB",enbH);
  	cmd.AddValue("ueD", "Distance*i between the i_th UE and eNodeB",ueD);
    cmd.AddValue("velocity","Velocity of Node",velocity);
    cmd.AddValue("distance","Distance of Movement",distance);
    cmd.AddValue("newVelocity","New Velocity after a few second", newVelocity);
    cmd.AddValue("newDistance","The distance to change the velocity", newDistance);
  	cmd.Parse(argc, argv);

	  ConfigStore inputConfig;
  	inputConfig.ConfigureDefaults ();

  	// parse again so you can override default values from the command line
  	cmd.Parse (argc, argv);	

	  //Log Enable
	  //  LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
	  //  LogComponentEnable ("UdpClient", LOG_LEVEL_ALL);
	  //  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
	  //  LogComponentEnable ("PacketSink", LOG_LEVEL_ALL);	

	  std::cout << "1. Configuring. Done!" << std::endl; 
}

void LteEpcSim::InstallEPC()
{
	  lteHelper = CreateObject<LteHelper> ();
  	epcHelper = CreateObject<EpcHelper> ();

  	lteHelper->SetEpcHelper (epcHelper);
  	lteHelper->SetSchedulerType("ns3::RrFfMacScheduler");
  	//lteHelper->SetSchedulerAttribute ("CqiTimerThreshold", UintegerValue (0.001));

  	//Pathloss Models
  	lteHelper->SetAttribute ("PathlossModel",
                               StringValue ("ns3::FriisPropagationLossModel"));
    //Create P-GW
	  pgw = epcHelper->GetPgwNode ();

    // Create a single RemoteHost
  	remoteHostContainer.Create (1);
  	remoteHost = remoteHostContainer.Get (0);
  	internet.Install (remoteHostContainer);
	
	  // Create the Internet
	  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
	  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
	  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010))); 
	  internetDevices = p2ph.Install (pgw, remoteHost);
	
	  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
	  internetIpIfaces = ipv4h.Assign (internetDevices);
	  // interface 0 is localhost, 1 is the p2p device
	  //remoteHostAddr = internetIpIfaces.GetAddress (1);
	
	  remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
	  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
	  
	  std::cout << "2. Installing LTE+EPC+remotehost. Done!" << std::endl; 
}

void LteEpcSim::InstallENBUEs()
{
	  //Create eNodeBs and UEs
  	enbNodes.Create(numberOfeNodeBs);
  	for (uint32_t i = 0; i < numberOfeNodeBs; i++)
    	{
      		NodeContainer iueNodes;
      		iueNodes.Create (numberOfUEs);
      		ueNodes.push_back (iueNodes);
    	}
    
    //Install Mobility Models on eNodeBs and UEs	
    InstallMobility();
    	
	  //1. Install LTE Devices to the nodes
	  //2. Attach one UE per eNodeB; 
    //3. Install the IP stack on the UEs attached to per eNodeB; 
    //4. Assign IP address to UEs.
	  
	  enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
	  
	  for (uint16_t i = 0; i < numberOfeNodeBs; i++)
		  {
			    NetDeviceContainer iueLteDevs;
		      iueLteDevs=lteHelper->InstallUeDevice (ueNodes.at(i));
		    	ueLteDevs.push_back(iueLteDevs);
		    	//1. Attach each uelteDev to per eNodeB
		    	for (uint16_t j=0; j < numberOfUEs; j++)
		      	{
		        		lteHelper->Attach (iueLteDevs.Get(j), enbLteDevs.Get(i));  
		      	}
		      
		    	//2. Install the IP stack on the UEs
		    	internet.Install (ueNodes.at(i));
		    
		    	Ipv4InterfaceContainer iueIpIface;
		    	iueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (iueLteDevs));
		    	ueIpIface.push_back(iueIpIface);
		    
		    	// Assign IP address to UEs, and install applications
		    	for (uint32_t u = 0; u < ueNodes.at(i).GetN (); ++u)
		      	{
		        		Ptr<Node> ueNode = ueNodes.at(i).Get (u);
		        		// Set the default gateway for the UE
		        		Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
		        		ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
		      	}
		    	lteHelper->ActivateEpsBearer (iueLteDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), EpcTft::Default ());
		  }
		  std::cout << "4. Installing eNodeBs: " << numberOfeNodeBs << "; UEs: "<< numberOfUEs*numberOfeNodeBs << ". Done!" << std::endl; 
}

void LteEpcSim::InstallMobility()
{
		// Install Mobility Model of eNodeBs
	  
	  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	  positionAlloc = CreateObject<ListPositionAllocator> ();
	  
	  for (uint16_t i = 0; i < numberOfeNodeBs; i++)
		{
		  Vector v(enbD * i, 500, enbH);
		  positionAlloc->Add (v);
		  enbPosition.push_back (v);
		}
	  mobility.SetPositionAllocator(positionAlloc);
	  mobility.Install(enbNodes);
	  
	  //Install Mobility Model of UEs
	  const ns3::Vector3D speed(velocity, 0.0, 0.0);	//Define the default velocity of ConstantVelocityMobilityModel
	  mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel","Velocity",Vector3DValue(speed));
	  //initial position
	  for (uint16_t i=0;i<numberOfeNodeBs;i++)
		{
		  positionAlloc = CreateObject<ListPositionAllocator> ();
		  for (uint16_t j = 0; j < numberOfUEs; j++)
		    {
		      positionAlloc->Add (Vector (enbPosition.at(i).x+ueD*j, enbPosition.at(i).y, 0));
		      mobility.SetPositionAllocator (positionAlloc);
		    }
		  mobility.Install(ueNodes.at(i));
		}
	  //Define Velocity for each UE
	  VMM_0_0 = ueNodes.at(0).Get(0) -> GetObject<ConstantVelocityMobilityModel>();
	  VMM_0_0 -> SetVelocity(Vector3D(velocity,0.0,0.0));
	  /*
	  std::cout << ueNodes.at(0).Get(0)->GetId() << std::endl;
    std::ostringstream oss;
	  oss << 
	  "/NodeList/" << ueNodes.at(0).Get(0)->GetId() << 
	  "/$ns3::ConstantVelocityMobilityModel/CourseChange";
	  Config::Connect (oss.str(),MakeCallback (&CourseChange));*/
	  
	  std::cout << "3. Installing Mobility Models. Velocity="<< velocity <<". Distance="<< distance<<". Done!" << std::endl; 
}

void LteEpcSim::InstallApplications()
{
	  // Install and start applications on UEs and remote host
    	uint16_t dlPort = 1234;
   // uint16_t ulPort = 1235;
   // uint16_t otherPort = 1236;
   // PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
    //PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
    //PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
    //ApplicationContainer clientApps;
    //ApplicationContainer serverApps;
    
    //Setup Servers on UEs or Remotehost
    //1. Servers on UEs
    //serverApps.Add (dlPacketSinkHelper.Install (ueNodes.at(0).Get(0)));
    
    //2. Setup udp client to UE_0 attached to eNodeB_0
    //UdpClientHelper dlClient (ueIpIface.at(0).GetAddress (0), dlPort);
    //dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
    //dlClient.SetAttribute ("MaxPackets", UintegerValue(3000));
    //dlClient.SetAttribute ("PacketSize", UintegerValue (1000));
    //clientApps.Add (dlClient.Install (remoteHost));
    ApplicationContainer onOffApp;
    
    OnOffHelper onOffHelper("ns3::UdpSocketFactory", InetSocketAddress(ueIpIface.at(0).GetAddress (0), dlPort));
    onOffHelper.SetAttribute("OnTime", RandomVariableValue(ConstantVariable(30)));
    onOffHelper.SetAttribute("OffTime", RandomVariableValue(ConstantVariable(0)));
    onOffHelper.SetAttribute("DataRate", DataRateValue(DataRate("2Mbps")));
    onOffHelper.SetAttribute("PacketSize", UintegerValue(1000));
    onOffApp.Add(onOffHelper.Install(remoteHost));
    onOffApp.Start(Seconds(0.01));
    onOffApp.Stop(Seconds(30));
  /*
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get(u)));
      serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
      serverApps.Add (packetSinkHelper.Install (ueNodes.Get(u)));

      UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
      dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
      dlClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

      UdpClientHelper ulClient (remoteHostAddr, ulPort);
      ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
      ulClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

      UdpClientHelper client (ueIpIface.GetAddress (u), otherPort);
      client.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
      client.SetAttribute ("MaxPackets", UintegerValue(1000000));

      clientApps.Add (dlClient.Install (remoteHost));
      clientApps.Add (ulClient.Install (ueNodes.Get(u)));
      if (u+1 < ueNodes.GetN ())
        {
          clientApps.Add (client.Install (ueNodes.Get(u+1)));
        }
      else
        {
          clientApps.Add (client.Install (ueNodes.Get(0)));
        }
    }
    */
    
 // std::stringstream oss;
 // internetIpIfaces.GetAddress (0).Print(oss);
 // NS_LOG_UNCOND (oss.str());
    
   
 // serverApps.Start (Seconds (0.01));
 // serverApps.Stop (Seconds(simTime));
 // clientApps.Start (Seconds (0.01));
 // clientApps.Stop (Seconds(simTime));
    std::cout << "5. Installing Applications. Done!" << std::endl; 
}

void LteEpcSim::SetFlowMonitor()
{
	  //Setup FlowMonitor
     monitor = fl.Install(ueNodes.at(0));
     monitor = fl.Install(ueNodes.at(1));
     monitor = fl.Install(remoteHostContainer);
     monitor->SetAttribute ("DelayBinWidth",
                           DoubleValue(0.001));
     monitor->SetAttribute ("JitterBinWidth",
                           DoubleValue(0.001));
     monitor->SetAttribute ("PacketSizeBinWidth",
                           DoubleValue(20));

    
    //Trace Movement of UEs
    /*
    std::ostringstream oss;
	  oss << 
	  "/NodeList/" << ueNodes.at(0).Get (0)->GetId() << 
	  "/$ns3::MobilityModel/CourseChange";*/
	  //Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",MakeCallback (&CourseChange));
	  
	  std::cout << "6. Setting FlowMonitor. Done!" << std::endl; 
}

static void getMobility (Ptr<Node> node)
{
  Ptr<ConstantVelocityMobilityModel> nodemodel= node -> GetObject<ConstantVelocityMobilityModel>();
  Vector position = nodemodel->GetPosition ();
  Vector velocity = nodemodel->GetVelocity ();
  std::cout << Simulator::Now().GetSeconds () << ": x="<< position.x<< ",y=" << position.y <<", v="<<velocity.x<<std::endl;
  
  Simulator::Schedule(Seconds (1.0), &getMobility, node);
}
/**
static void setNewVelocity (Ptr<Node> node, Vector3D V)
{
  Ptr<ConstantVelocityMobilityModel> nodemodel= node -> GetObject<ConstantVelocityMobilityModel>();
  nodemodel -> SetVelocity(V);
  Vector position = nodemodel->GetPosition ();
  std::cout << Simulator::Now().GetSeconds () << ": The Velocity has been changed to :" << V.x << "From: "<< position.x <<"m."<<std::endl;
}
*/
int LteEpcSim::Run()
{
	  InstallEPC();
	  InstallENBUEs();
	  
	  InstallApplications();
	  
    //Tracing
    lteHelper->EnableTraces ();
    //Uncomment to enable PCAP tracing
    //p2ph.EnablePcapAll("lena-epc-first");
    
    
    
    SetFlowMonitor();
    
    std::cout << "7. Simulation is running..." << std::endl;
    
    Simulator::Schedule (Seconds (1.0), &getMobility, ueNodes.at(0).Get(0));
    //Simulator::Schedule (Seconds (15), &setNewVelocity, ueNodes.at(0).Get(0), Vector3D(newVelocity,0.0,0.0));
    
    
    
    Simulator::Stop(Seconds(30));
    
    Simulator::Run();
    
    monitor->SerializeToXmlFile("result1.xml",true,true);
    	/*GtkConfigStore config;
    	config.ConfigureAttributes();*/
    Simulator::Destroy();
    std::cout << "8. Simulation is Done!" << std::endl; 
	  return 0;
}



int
main (int argc, char *argv[])
{
	  LteEpcSim lte;
	  lte.Configure(argc, argv);
	  return lte.Run();
}

