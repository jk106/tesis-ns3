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
 * Author: Marco Miozzo <marco.miozzo@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 */


#include "mih-tag.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (MihTag);

TypeId
MihTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MihTag")
    .SetParent<Tag> ()
    .AddConstructor<MihTag> ()
  ;
  return tid;
}

TypeId
MihTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

MihTag::MihTag ()
{
}

MihTag::~MihTag ()
{
}

uint32_t
MihTag::GetSerializedSize (void) const
{
  return 1+sizeof(double);
}

void
MihTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_command);
  i.WriteDouble(m_parameter);
}

void
MihTag::Deserialize (TagBuffer i)
{
  m_command = i.ReadU8 ();
  m_parameter = i.ReadDouble();
}

void
MihTag::Print (std::ostream &os) const
{
  os << m_command<<" "<<m_parameter;
}

void
MihTag::SetCommand(uint8_t value)
{
  m_command=value;
}

uint8_t
MihTag::GetCommand () const
{
  return m_command;
}

void
MihTag::SetParameter(double value)
{
  m_parameter=value;
}

double
MihTag::GetParameter () const
{
  return m_parameter;
}

} // namespace ns3
