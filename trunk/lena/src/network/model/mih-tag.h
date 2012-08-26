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
 * Author: Marco Miozzo  <marco.miozzo@cttc.es>
 *         Nicola Baldo  <nbaldo@cttc.es>
 */
#ifndef MIH_TAG_H
#define MIH_TAG_H

#include "ns3/tag.h"

namespace ns3 {

/**
 * Tag used to define PHY parameters 
 */
class MihTag : public Tag
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Create an empty LtePhyTag
   */
  MihTag ();

  
  virtual ~MihTag ();

  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual uint32_t GetSerializedSize () const;
  virtual void Print (std::ostream &os) const;
  void SetCommand(uint8_t value);
  uint8_t GetCommand () const;
  void SetParameter(double value);
  double GetParameter () const;

private:
  uint8_t m_command;
  double m_parameter;
};



} // namespace ns3

#endif /* LTE_PHY_TAG_H */
