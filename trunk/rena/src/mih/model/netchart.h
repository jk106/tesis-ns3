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
#ifndef NETCHART_H
#define NETCHART_H

#include "ns3/object.h"
#include "ns3/net-device.h"
#include "ns3/mac48-address.h"
#include <stdint.h>
#include <string>
#include "ns3/traced-callback.h"
#include <vector>
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
#include "ns3/mobility-module.h"




namespace ns3 {

class SimpleChannel;
class Node;

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
class NetChart : public Object
{
public:
  static TypeId GetTypeId (void);
  NetChart ();

  void AddRouting(Ipv4Address ipv4a);
  void RemoveRouting(Ipv4Address ipv4a);
  void SetId(uint8_t id);
  void SetNodes(std::vector<Ptr<NetDevice> > &c, std::vector<int> &d, std::vector<int> &e);
  uint8_t GetId();
  void SetTechnology(uint8_t tech);
  void SetApLocation(Vector a);
  uint8_t GetTechnology();
  Vector GetApLocation();

protected:
  virtual void DoDispose (void);
private:
  uint8_t m_id;  
  std::vector<Ptr<Node> > m_nodes;//The nodes of this network
  std::vector<int> m_indexesdown;//The interfaces of each node downstream
  std::vector<int> m_indexesup;//The interfaces of each node upstream
  uint8_t m_tech;
  Vector m_aplocation;
};

} // namespace ns3

#endif /* SIMPLE_NET_DEVICE_H */
