/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "mih-net-device.h"
#include "ns3/simple-channel.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/error-model.h"
#include "ns3/mobility-module.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/mih-tag.h"
#include "ns3/vector.h"


NS_LOG_COMPONENT_DEFINE ("MihNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (MihNetDevice);

int McsToItbss[29] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 9, 10, 11, 12, 13, 14, 15, 15, 16, 17, 18,
  19, 20, 21, 22, 23, 24, 25, 26
};

int rates[27]={ 3112, 4008, 4968, 6456, 7992, 9528, 11448, 13536, 15264, 17568, 19080, 22152, 25456, 28336, 31704, 34008, 35160, 39232, 43816, 46888, 51024, 55056, 59256, 63776, 66592, 71112, 75376};


TypeId 
MihNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MihNetDevice")
    .SetParent<NetDevice> ()
    .AddConstructor<MihNetDevice> ()
  ;
  return tid;
}

MihNetDevice::MihNetDevice ()
  : m_channel (0),
    m_node (0),
    m_mtu (0xffff),
    m_ifIndex (0),
    m_active(0),
    p_wifi(20000),
    p_wimax(10000),
    p_lte(15000),
    hop_wifi(false),
    hop_wimax(false),
    hop_lte(false),
    timeout_wimax(-3),
    m_dependent(false),
    m_qos(1),
    m_timeout(0)
{
}

bool
MihNetDevice::NonPromiscReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                                const Address &from)
{
  NS_LOG_FUNCTION (this);
  MihTag tag;
  if(packet->PeekPacketTag(tag))
  {
    if(m_active==1)
{
    UpdateParameter(tag.GetCommand(),tag.GetParameter());
    //return true;
}
else
{
    UpdateParameter(tag.GetCommand(),tag.GetParameter());
    return true;
}
  }

  if(device == m_devices[m_active])
  {
     m_rxCallback (this, packet, protocol, from);
  }
  else
  {
  //NS_LOG_DEBUG(this<<"Receive!!");  
  }

  return true;
}

void
MihNetDevice::UpdateParameter(uint8_t command, double parameter)
{
  if(command==1)
  {
     //NS_LOG_DEBUG(this<<"Recibido de WiFi "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s");
if(GetNode()->GetId()==2)
NS_LOG_DEBUG(this<<"2Recibido de WiFi "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s");
if(GetNode()->GetId()==0)
NS_LOG_DEBUG(this<<"0Recibido de WiFi "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s");
     if(parameter<1200&&parameter >800)
     {
       NS_LOG_DEBUG(this<<"Perdiendo WiFi "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s");
       hop_wifi=true;
     }
     else if(parameter<800)
     {
       NS_LOG_DEBUG(this<<"Wifi Perdido "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s");
       p_wifi=1;
hop_wifi=false;
     }
     else
     {
       p_wifi=parameter;
       hop_wifi=false;
     }
  }
  else if(command==2)
  {
     //NS_LOG_DEBUG(this<<"Recibido de WiMAX "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s");
     if(parameter<10000&&parameter >9870)
     {
       NS_LOG_DEBUG(this <<"Perdiendo WiMAX "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s");
       hop_wimax=true;
       timeout_wimax=Simulator::Now().GetSeconds();
     }
     else if(parameter<9870)
     {
       NS_LOG_DEBUG(this<<"WiMAX Perdido "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s");
       p_wimax=0;
hop_wimax=false;
     }
     else if(p_wimax==0)
       p_wimax=0;
     else
     {
       p_wimax=parameter;
       if((Simulator::Now().GetSeconds()-timeout_wimax)>2)
         hop_wimax=false;
     }
  }
  else if(command==3)
  {
     //NS_LOG_DEBUG(this<<"Recibido de LTE "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s");
     int mcsd=parameter/10000;
     int mcsu=(parameter-mcsd*10000)/100;
     double error=(parameter-mcsd*10000-mcsu*100);
     if(mcsu==0&&mcsd==16)
     {
       NS_LOG_DEBUG(this<<"Perdiendo LTE "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s");
       hop_lte=true;
     }
     else if((mcsu==0&&mcsd<16)||(mcsu==28&&mcsd==14))//We lost it because this is an inplausible situation.
     {
       NS_LOG_DEBUG(this<<"LTE Perdido "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s");
       p_lte=0;
hop_lte=false;
     }
     else
     {
       if(mcsu<mcsd)
         p_lte=(1-error)*rates[McsToItbss[mcsu]]/5;
       else
         p_lte=(1-error)*rates[McsToItbss[mcsd]]/5;
       hop_lte=false;
     }
  }
  else
  {
     NS_LOG_DEBUG(this<<"Recibido de WTF "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s");
  }
  if(Simulator::Now().GetSeconds()>10)
    eval();
}

void
MihNetDevice::SetNetId(uint8_t netid)
{
  m_netid=netid;
  net->RequestPSol(m_netid,this,m_active,4);
}

void
MihNetDevice::UpdateNetId(uint8_t netid)
{
  m_netid=netid;
}

uint8_t
MihNetDevice::GetNetId()
{
  return m_netid;
}

void
MihNetDevice::SetQos(uint8_t qos)
{
  m_qos=qos;
}

uint8_t
MihNetDevice::GetQos()
{
  return m_qos;
}

void
MihNetDevice::eval()
{
  if(m_timeout+10<Simulator::Now().GetSeconds())
{
  if(!m_dependent)
{
  double clte=p_lte/4;
  double cwimax=p_wimax/2;
  Vector speed=this->GetNode()->GetObject<MobilityModel>()->GetVelocity();
  double velocity=sqrt(speed.x*speed.x+speed.y*speed.y+speed.z*speed.z);
  if(p_wifi>cwimax && p_wifi>clte && clte>cwimax)
  {
    //NS_LOG_DEBUG(this<<"Wifi "<<p_wifi<<", Lte "<<clte<<", Wimax "<<cwimax<<GetNode()->GetId());
    if(hop_wifi && m_active!=3)
    {
      //Activate(3);
      net->RequestPSol(m_netid,this,m_active,3);
      std::cout << Simulator::Now().GetSeconds () << ": Device Swapped to LTE for HOP "<<GetNode()->GetId()<<std::endl;
m_timeout=Simulator::Now().GetSeconds();
    }
    else if(m_active!=1 && velocity<5 && !hop_wifi)
    {
      //Activate(1);
      net->RequestPSol(m_netid,this,m_active,1);
      std::cout << Simulator::Now().GetSeconds () << ": Device Swapped to WiFi1" <<GetNode()->GetId()<<std::endl;
m_timeout=Simulator::Now().GetSeconds();

    }
  }
  else if(p_wifi>cwimax && p_wifi>clte && clte<=cwimax)
  {
    //NS_LOG_DEBUG(this<<"Wifi "<<p_wifi<<", Wimax "<<cwimax<<", Lte "<<clte<<GetNode()->GetId());
    if(hop_wifi &&m_active!=2)
    {
      //Activate(2);
      net->RequestPSol(m_netid,this,m_active,2);
      std::cout << Simulator::Now().GetSeconds () << ": Device Swapped to WiMAX for HOP"<<std::endl;
m_timeout=Simulator::Now().GetSeconds();

    }
    else if(m_active!=1 && velocity<5 && !hop_wifi)
    {
      //Activate(1);
      net->RequestPSol(m_netid,this,m_active,1);
      std::cout << Simulator::Now().GetSeconds () << ": Device Swapped to WiFi2 "<<GetNode()->GetId()<<std::endl;
m_timeout=Simulator::Now().GetSeconds();

    }
  }
  else if(p_wifi<cwimax && p_wifi>clte)
  {
    //NS_LOG_DEBUG(this<<"Wimax "<<cwimax<<", Wifi "<<p_wifi<<", Lte "<<clte<<GetNode()->GetId());
    if(hop_wimax && m_active!=1)
    {
      //Activate(1);
      net->RequestPSol(m_netid,this,m_active,1);
      std::cout << Simulator::Now().GetSeconds () << ": Device Swapped to WiFi for HOP1"<<std::endl;
m_timeout=Simulator::Now().GetSeconds();

    }
    else if(m_active!=2 && !hop_wimax)
    {
      //Activate(2);
      net->RequestPSol(m_netid,this,m_active,2);
      std::cout << Simulator::Now().GetSeconds () << ": Device Swapped to WiMAX1 "<<GetNode()->GetId()<<std::endl;
m_timeout=Simulator::Now().GetSeconds();

    }

  }
  else if(p_wifi>cwimax && p_wifi<clte)
  {
    //NS_LOG_DEBUG(this<<"Lte "<<clte<<", Wifi "<<p_wifi<<", Wimax "<<cwimax);
    if(hop_lte && m_active!=1)
    {
      //Activate(1);
      net->RequestPSol(m_netid,this,m_active,1);
      std::cout << Simulator::Now().GetSeconds () << ": Device Swapped to WiFi for HOP2"<<std::endl;
m_timeout=Simulator::Now().GetSeconds();

    }
    if(m_active!=3 && !hop_lte)
    {
      //Activate(3);
      net->RequestPSol(m_netid,this,m_active,3);
      std::cout << Simulator::Now().GetSeconds () << ": Device Swapped to LTE "<<GetNode()->GetId()<<std::endl;
m_timeout=Simulator::Now().GetSeconds();

    }
    
  }
  else if(p_wifi<cwimax && cwimax>clte && p_wifi<clte)
  {
    //NS_LOG_DEBUG(this<<"Wimax "<<cwimax<<", Lte "<<clte<<", Wifi "<<p_wifi);
    if(hop_wimax && m_active!=3)
    {
      //Activate(3);
      net->RequestPSol(m_netid,this,m_active,3);
      std::cout << Simulator::Now().GetSeconds () << ": Device Swapped to LTE for HOP "<<GetNode()->GetId()<<std::endl;
m_timeout=Simulator::Now().GetSeconds();

    }
    if(m_active!=2 && !hop_wimax)
    {
      //Activate(2);
      net->RequestPSol(m_netid,this,m_active,2);
      std::cout << Simulator::Now().GetSeconds () << ": Device Swapped to WiMAX2 "<<GetNode()->GetId()<<std::endl;
m_timeout=Simulator::Now().GetSeconds();

    }

  }
  else if(p_wifi<cwimax && p_wifi<clte && cwimax<clte)
  {
    //NS_LOG_DEBUG(this<<"Lte "<<clte<<" Wimax "<<cwimax<<", Wifi "<<p_wifi);
    if(hop_lte &&m_active!=2)
    {
      //Activate(2);
      net->RequestPSol(m_netid,this,m_active,2);
      std::cout << Simulator::Now().GetSeconds () << ": Device Swapped to WiMAX for HOP"<<std::endl;
m_timeout=Simulator::Now().GetSeconds();

    }
    if(m_active!=3 && !hop_lte)
    {
      //Activate(3);
      net->RequestPSol(m_netid,this,m_active,3);
      std::cout << Simulator::Now().GetSeconds () << ": Device Swapped to LTE "<<GetNode()->GetId()<<std::endl;
m_timeout=Simulator::Now().GetSeconds();

    }
  }
}
  else if(hop_wifi && m_active==1)
{
  net->RequestPSol(m_netid,this,m_active,0);
  std::cout << Simulator::Now().GetSeconds () << ": Device Swapped from Wi-Fi "<<GetNode()->GetId()<<std::endl;
m_timeout=Simulator::Now().GetSeconds();

}
  else if(hop_wimax && m_active==2)
{
  net->RequestPSol(m_netid,this,m_active,0);
  std::cout << Simulator::Now().GetSeconds () << ": Device Swapped from WiMAX"<<std::endl;
m_timeout=Simulator::Now().GetSeconds();

}
  else if(hop_lte && m_active==3)
{
  net->RequestPSol(m_netid,this,m_active,0);
  std::cout << Simulator::Now().GetSeconds () << ": Device Swapped from LTE"<<std::endl;
m_timeout=Simulator::Now().GetSeconds();

}
}

}

void
MihNetDevice::Receive (Ptr<Packet> packet, uint16_t protocol,
                          Mac48Address to, Mac48Address from)
{
  
  
}

void
MihNetDevice::Activate(uint8_t index)
{
  m_devices[m_active]->SetReceiveCallback (MakeCallback (&MihNetDevice::NonPromiscReceiveFromDevice, this));
  m_active=index;
  if(m_devices[m_active]->NeedsArp())
  {
    m_node->Appropiate(m_devices[m_active]);

  }
Ptr<NetDevice> device = m_devices[m_active];

    Ptr<Node> node = device->GetNode ();
    NS_ASSERT_MSG (node, "Ipv4AddressHelper::Assign(): NetDevice is not not associated "
                   "with any node -> fail");

    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    NS_ASSERT_MSG (ipv4, "Ipv4AddressHelper::Assign(): NetDevice is associated"
                   " with a node without IPv4 stack installed -> fail "
                   "(maybe need to use InternetStackHelper?)");

    int32_t interface = ipv4->GetInterfaceForDevice (device);
    if (interface == -1)
    {
      interface = ipv4->AddInterface (device);
    }
    NS_ASSERT_MSG (interface >= 0, "Ipv4AddressHelper::Assign(): "
                   "Interface index not found");

    Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress (mih_address, "255.255.255.0");
    ipv4->AddAddress (interface, ipv4Addr);
    ipv4->SetMetric (interface, 1);
    ipv4->SetUp (interface);
}

void
MihNetDevice::SetNetDevices(std::vector<Ptr<NetDevice> > &c)
{
  for(unsigned int i=0; i<c.size();i++)
  {
     Ptr<NetDevice> dev= c[i];
     m_devices.push_back(dev);
     dev->SetReceiveCallback (MakeCallback (&MihNetDevice::NonPromiscReceiveFromDevice, this));
  }
}

void
MihNetDevice::SetAddress(Ipv4Address addr)
{
  mih_address=addr;
}

void 
MihNetDevice::SetIfIndex (const uint32_t index)
{
  m_ifIndex = index;
}
uint32_t 
MihNetDevice::GetIfIndex (void) const
{
  return m_ifIndex;
}
Ptr<Channel> 
MihNetDevice::GetChannel (void) const
{
  return m_channel;
}
void
MihNetDevice::SetAddress (Address address)
{
  m_address = Mac48Address::ConvertFrom (address);
}
Address 
MihNetDevice::GetAddress (void) const
{
  //
  // Implicit conversion from Mac48Address to Address
  //
  return m_address;
}
bool 
MihNetDevice::SetMtu (const uint16_t mtu)
{
  m_mtu = mtu;
  return true;
}
uint16_t 
MihNetDevice::GetMtu (void) const
{
  return m_mtu;
}
bool 
MihNetDevice::IsLinkUp (void) const
{
  return true;
}
void 
MihNetDevice::AddLinkChangeCallback (Callback<void> callback)
{}
bool 
MihNetDevice::IsBroadcast (void) const
{
  return true;
}
Address
MihNetDevice::GetBroadcast (void) const
{
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}
bool 
MihNetDevice::IsMulticast (void) const
{
  return false;
}
Address 
MihNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  return Mac48Address::GetMulticast (multicastGroup);
}

Address MihNetDevice::GetMulticast (Ipv6Address addr) const
{
  return Mac48Address::GetMulticast (addr);
}

bool 
MihNetDevice::IsPointToPoint (void) const
{
  return false;
}

bool 
MihNetDevice::IsBridge (void) const
{
  return false;
}

bool 
MihNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (packet << dest << protocolNumber);
  //NS_LOG_DEBUG(this<<"Send!!");
  if(m_active!=1)
  m_devices[m_active]->Send(packet,dest,protocolNumber);
  else
  m_devices[m_active]->Send(packet,Mac48Address("AA:BB:CC:DD:EE:FF"),protocolNumber);
  return true;
}
bool 
MihNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  return true;
}

Ptr<Node> 
MihNetDevice::GetNode (void) const
{
  return m_node;
}
void 
MihNetDevice::SetNode (Ptr<Node> node)
{
  m_node = node;
}
bool 
MihNetDevice::NeedsArp (void) const
{
  return false;
}
void 
MihNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  m_rxCallback = cb;
}

void
MihNetDevice::DoDispose (void)
{
  m_channel = 0;
  m_node = 0;
  NetDevice::DoDispose ();
}


void
MihNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  m_promiscCallback = cb;
}

bool
MihNetDevice::SupportsSendFrom (void) const
{
  return true;
}

void
MihNetDevice::SetNetworkManager(Ptr<NetworkManager> netman)
{
  net=netman;
}

Ipv4Address
MihNetDevice::GetMihAddress()
{
  return mih_address;
}

void 
MihNetDevice::SetDependent(bool dep)
{
  m_dependent=dep;
  if(dep==true)
  {
    p_wifi=0;
    p_wimax=0;
    p_lte=0;
  }
}

} // namespace ns3
