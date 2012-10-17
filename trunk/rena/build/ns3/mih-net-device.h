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
#ifndef MIH_NET_DEVICE_H
#define MIH_NET_DEVICE_H

#include "ns3/net-device.h"
#include "ns3/network-manager.h"
#include "ns3/mac48-address.h"
#include <stdint.h>
#include <string>
#include "ns3/traced-callback.h"
#include <vector>
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"



namespace ns3 {

class SimpleChannel;
class Node;
class NetworkManager;

/**
 * \ingroup netdevice
 *
 * This device does not have a helper and assumes 48-bit mac addressing;
 * the default address assigned to each device is zero, so you must 
 * assign a real address to use it.  There is also the possibility to
 * add an ErrorModel if you want to force losses on the device.
 * 
 * \brief simple net device for simple things and testing
 */
class MihNetDevice : public NetDevice
{
public:
  static TypeId GetTypeId (void);
  MihNetDevice ();

  bool NonPromiscReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet>, uint16_t protocol,
                                 const Address &from);
  void Receive (Ptr<Packet> packet, uint16_t protocol, Mac48Address to, Mac48Address from);
  void SetNetDevices(std::vector<Ptr<NetDevice> > &c);
  void Activate(uint8_t index);
  void SetNetId(uint8_t netid);
  void SetDependent(bool dep);
  uint8_t GetNetId();
  void SetQos(uint8_t qos);
  uint8_t GetQos();
  void SetAddress(Ipv4Address addr);
  void eval();
  Ipv4Address GetMihAddress();
  void SetNetworkManager(Ptr<NetworkManager> netman);
  virtual void UpdateParameter(uint8_t command, double parameter);
  
  // inherited from NetDevice base class.
  virtual void SetIfIndex (const uint32_t index);
  virtual uint32_t GetIfIndex (void) const;
  virtual Ptr<Channel> GetChannel (void) const;
  virtual void SetAddress (Address address);
  virtual Address GetAddress (void) const;
  virtual bool SetMtu (const uint16_t mtu);
  virtual uint16_t GetMtu (void) const;
  virtual bool IsLinkUp (void) const;
  virtual void AddLinkChangeCallback (Callback<void> callback);
  virtual bool IsBroadcast (void) const;
  virtual Address GetBroadcast (void) const;
  virtual bool IsMulticast (void) const;
  virtual Address GetMulticast (Ipv4Address multicastGroup) const;
  virtual bool IsPointToPoint (void) const;
  virtual bool IsBridge (void) const;
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);
  virtual bool NeedsArp (void) const;
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);

  virtual Address GetMulticast (Ipv6Address addr) const;

  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom (void) const;

protected:
  virtual void DoDispose (void);
private:
  Ptr<SimpleChannel> m_channel;
  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscCallback;
  Ptr<Node> m_node;
  uint16_t m_mtu;
  uint32_t m_ifIndex;
  Mac48Address m_address;
  uint8_t m_active;
  double p_wifi;
  double p_wimax;
  double p_lte;
  bool hop_wifi;
  bool hop_wimax;
  bool hop_lte;
  int timeout_wimax;
  Ipv4Address mih_address;
  std::vector<Ptr<NetDevice> > m_devices;
  Ptr<NetworkManager> net;
  uint8_t m_netid;
  bool m_dependent;
  uint8_t m_qos;
};

} // namespace ns3

#endif /* SIMPLE_NET_DEVICE_H */
