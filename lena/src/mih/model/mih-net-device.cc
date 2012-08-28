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
#include "ns3/trace-source-accessor.h"
#include "ns3/mih-tag.h"

NS_LOG_COMPONENT_DEFINE ("MihNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (MihNetDevice);

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
    m_active(0)
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
    UpdateParameter(tag.GetCommand(),tag.GetParameter());
    return true;
  }

  if(device == m_devices[m_active])
  {
     m_rxCallback (this, packet, protocol, from);
  }
  else
  {
  std::cout<<"Receive!!"<<std::endl;  
  }

  return true;
}

void
MihNetDevice::UpdateParameter(uint8_t command, double parameter)
{
  if(command==1)
  {
     //std::cout<<"Recibido de WiFi "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s"<<std::endl;
     if(parameter<1200&&parameter >800)
     {
       std::cout<<"Perdiendo WiFi "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s"<<std::endl;
     }
     if(parameter<800)
     {
       std::cout<<"Wifi Perdido "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s"<<std::endl;
     }
  }
  else if(command==2)
  {
     //std::cout<<"Recibido de WiMAX "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s"<<std::endl;
     if(parameter<10000&&parameter >9970)
     {
       std::cout<<"Perdiendo WiMAX "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s"<<std::endl;
     }
     if(parameter<9870)
     {
       std::cout<<"WiMAX Perdido "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s"<<std::endl;
     }
  }
  else
  {
     std::cout<<"Recibido de WTF "<<parameter<<" en tiempo "<<Simulator::Now().GetSeconds()<<" s"<<std::endl;
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
  std::cout<<"Send!!"<<std::endl;
  m_devices[m_active]->Send(packet,dest,protocolNumber);
  return true;
}
bool 
MihNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  std::cout<<"SendFrom!!"<<std::endl;
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

} // namespace ns3
