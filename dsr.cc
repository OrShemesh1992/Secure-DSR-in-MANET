
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/dsr-helper.h"
#include "ns3/dsr-main-helper.h"
#include "ns3/animation-interface.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/dsr-rcache.h"
using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhocGrid");

void ReceivePacket (Ptr<Socket> socket)
{
  while (socket->Recv ())
    {
      NS_LOG_UNCOND ("Received one packet!");
    }
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {
      //****************************add option DsrSendBuffer

      dsr::DsrSendBuffer q;
      q.SetMaxQueueLen (32);
       q.SetSendBufferTimeout (Seconds (30));
       Ptr<const Packet> packet = Create<Packet> ();
       Ipv4Address dst1 = Ipv4Address ("10.1.1.5");
       dsr::DsrSendBuffEntry e1 (packet, dst1, Seconds (30.2));
       q.Enqueue (e1);
       q.Enqueue (e1);
       q.Enqueue (e1);
       q.DropPacketWithDst (Ipv4Address ("10.1.1.5"));

      Ipv4Address dst2 = Ipv4Address ("10.1.1.2");
      dsr::DsrSendBuffEntry e2 (packet, dst2, Seconds (30));
      q.Enqueue (e1);
      q.Enqueue (e2);
      Ptr<Packet> packet2 = Create<Packet> ();
      dsr::DsrSendBuffEntry e3 (packet2, dst2, Seconds (30));
      q.Enqueue (e2);
      q.Enqueue (e3);
      Ptr<Packet> packet4 = Create<Packet> ();
      Ipv4Address dst4 = Ipv4Address ("10.1.1.4");
      dsr::DsrSendBuffEntry e4 (packet4, dst4, Seconds (20));
      q.Enqueue (e4);
      q.DropPacketWithDst (Ipv4Address ("10.1.1.4"));

      //CheckSizeLimit ();

      // Simulator::Schedule (q.GetSendBufferTimeout () + Seconds (30), &DsrSendBuffTest::CheckTimeout, this);
      //
      // Simulator::Run ();
      // Simulator::Destroy ();







      //****************************add option rcache
      Ptr<dsr::DsrRouteCache> rcache = CreateObject<dsr::DsrRouteCache> ();
      std::vector<Ipv4Address> ip;
      ip.push_back (Ipv4Address ("10.1.1.1"));
      ip.push_back (Ipv4Address ("10.1.1.2"));
      Ipv4Address dst = Ipv4Address ("10.1.1.2");
      dsr::DsrRouteCacheEntry entry (ip, dst, Seconds (30));
      // NS_TEST_EXPECT_MSG_EQ (entry.GetVector ().size (), 2, "trivial");
      // NS_TEST_EXPECT_MSG_EQ (entry.GetDestination (), Ipv4Address ("0.0.0.1"), "trivial");
      // NS_TEST_EXPECT_MSG_EQ (entry.GetExpireTime (), Seconds (1), "trivial");

      entry.SetExpireTime (Seconds (30));
      // NS_TEST_EXPECT_MSG_EQ (entry.GetExpireTime (), Seconds (3), "trivial");
      entry.SetDestination (Ipv4Address ("10.1.1.3"));
      //NS_TEST_EXPECT_MSG_EQ (entry.GetDestination (), Ipv4Address ("1.1.1.1"), "trivial");
      ip.push_back (Ipv4Address ("10.1.1.4"));
      entry.SetVector (ip);
    //  NS_TEST_EXPECT_MSG_EQ (entry.GetVector ().size (), 3, "trivial");
      std::cout << " AddRout?  " << rcache->AddRoute (entry)<<"\n";
      //NS_TEST_EXPECT_MSG_EQ (rcache->AddRoute (entry), true, "trivial");

      // std::vector<Ipv4Address> ip2;
      // ip2.push_back (Ipv4Address ("1.1.1.0"));
      // ip2.push_back (Ipv4Address ("1.1.1.1"));
      // Ipv4Address dst2 = Ipv4Address ("1.1.1.1");
      // dsr::DsrRouteCacheEntry entry2 (ip2, dst2, Seconds (2));
      // dsr::DsrRouteCacheEntry newEntry;
      // NS_TEST_EXPECT_MSG_EQ (rcache->AddRoute (entry2), true, "trivial");
      // NS_TEST_EXPECT_MSG_EQ (rcache->LookupRoute (dst2, newEntry), true, "trivial");
      // NS_TEST_EXPECT_MSG_EQ (rcache->DeleteRoute (Ipv4Address ("2.2.2.2")), false, "trivial");
      //
      // NS_TEST_EXPECT_MSG_EQ (rcache->DeleteRoute (Ipv4Address ("1.1.1.1")), true, "trivial");
      // NS_TEST_EXPECT_MSG_EQ (rcache->DeleteRoute (Ipv4Address ("1.1.1.1")), false, "trivial");
        //****************************add option DsrOptionAckHeader
      dsr::DsrOptionAckHeader AckHeader;

      AckHeader.SetRealSrc (Ipv4Address ("1.1.1.0"));
      AckHeader.SetRealDst (Ipv4Address ("1.1.1.1"));
      AckHeader.SetAckId (1);
        //****************************add option DsrOptionRerrUnreachHeader
      dsr::DsrOptionRerrUnreachHeader RerrUnreachHeader;
      RerrUnreachHeader.SetErrorSrc (Ipv4Address ("1.1.1.0"));
      RerrUnreachHeader.SetErrorDst (Ipv4Address ("1.1.1.1"));
      RerrUnreachHeader.SetSalvage (1);
      RerrUnreachHeader.SetUnreachNode (Ipv4Address ("1.1.1.2"));

      //****************************add option DsrOptionSRHeader
      dsr::DsrOptionSRHeader sr;
      std::vector<Ipv4Address> nodeList1;
      nodeList1.push_back (Ipv4Address ("1.1.1.0"));
      nodeList1.push_back (Ipv4Address ("1.1.1.1"));
      nodeList1.push_back (Ipv4Address ("1.1.1.2"));
      sr.SetNodesAddress (nodeList1);
      sr.SetSalvage (1);
      sr.SetSegmentsLeft (2);
      //****************************add option DsrOptionRrepHeader
       dsr::DsrOptionRrepHeader h;
       std::vector<Ipv4Address> nodeList;
       nodeList.push_back (Ipv4Address ("10.0.0.1"));
       nodeList.push_back (Ipv4Address ("10.0.0.2"));
       nodeList.push_back (Ipv4Address ("10.0.0.3"));
       nodeList.push_back (Ipv4Address ("10.0.0.4"));
       nodeList.push_back (Ipv4Address ("10.0.0.5"));
       nodeList.push_back (Ipv4Address ("10.0.0.6"));
       h.SetNodesAddress(nodeList);

      //****************************add option DsrOptionRreqHeader
      dsr::DsrOptionRreqHeader rreqHeader;
      rreqHeader.SetTarget(Ipv4Address ("10.0.0.3"));
      rreqHeader.SetNumberAddress(10);
      rreqHeader.SetTarget(Ipv4Address ("10.0.0.9"));
      rreqHeader.SetId(8);
      rreqHeader.AddNodeAddress(Ipv4Address ("10.0.0.1"));
      rreqHeader.AddNodeAddress(Ipv4Address ("10.0.0.2"));
      rreqHeader.AddNodeAddress(Ipv4Address ("10.0.0.3"));
      rreqHeader.AddNodeAddress(Ipv4Address ("10.0.0.4"));
      rreqHeader.SetNodeAddress(1,Ipv4Address ("10.0.0.1"));
      rreqHeader.SetNodeAddress(2,Ipv4Address ("10.0.0.2"));
      rreqHeader.SetNodeAddress(3,Ipv4Address ("10.0.0.3"));
      rreqHeader.SetNodeAddress(4,Ipv4Address ("10.0.0.4"));
      //****************************install option to header
      ns3::Packet::EnablePrinting();
      Ptr<Packet> p = Create<Packet> (pktSize);
      dsr::DsrRoutingHeader header;
      header.AddDsrOption (rreqHeader);
      //****************************add heder to packet
      p->AddHeader (header);
      p->RemoveAtStart (8);
      // p->AddHeader (header);
      // p->RemoveAtStart (8);
      std::cout << p->ToString();
      socket->Send (p);

      Simulator::Schedule (pktInterval, &GenerateTraffic,
                           socket, pktSize,pktCount - 1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}


int main (int argc, char *argv[])
{
  std::string phyMode ("DsssRate1Mbps");
  double distance = 500;  // m
  uint32_t packetSize = 1024; // bytes
  uint32_t numPackets = 1;
  uint32_t numNodes = 15;  // by default, 5x5
  uint32_t sinkNode = 5;
  uint32_t sourceNode = 9;
  double interval = 1.0; // seconds



  // Convert to time object
  Time interPacketInterval = Seconds (interval);

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));

  NodeContainer c;
  c.Create (numNodes);

//***************************wifi*************************
  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  // if (verbose)
    //{
      //wifi.EnableLogComponents ();  // Turn on all Wifi logging
     //}

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (-10) );
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add an upper mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);
  //***************************END**************************

//***************************SetPosition*************************
MobilityHelper mobilityAdhoc;
mobilityAdhoc.SetPositionAllocator ("ns3::GridPositionAllocator",
                             "MinX", DoubleValue (0.0),
                             "MinY", DoubleValue (0.0),
                             "DeltaX", DoubleValue (distance),
                             "DeltaY", DoubleValue (distance),
                             "GridWidth", UintegerValue (5),
                             "LayoutType", StringValue ("RowFirst"));
mobilityAdhoc.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
mobilityAdhoc.Install (c);
int jump=1,jump1=0,jump2=1;
for (uint n=0 ; n < c.GetN() ; n++)
 {
   if(n<4){
    Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
    mob->SetVelocity(Vector(0, 0, 0));
    mob->SetPosition(Vector(jump, 1.15, 0));
    jump++;

  }
  if(n>=4&&n<9){
   Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
   mob->SetVelocity(Vector(0, 0, 0));
   mob->SetPosition(Vector(jump1, 2.3, 0));
      jump1+=2;
 }
 if(n>=9){
  Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
  mob->SetVelocity(Vector(0, 0, 0));
  mob->SetPosition(Vector(jump2, 3.3, 0));
   jump2++;
}
 }
  // MobilityHelper mobilityAdhoc;
  // mobilityAdhoc.SetPositionAllocator ("ns3::GridPositionAllocator",
  //                              "MinX", DoubleValue (0.0),
  //                              "MinY", DoubleValue (0.0),
  //                              "DeltaX", DoubleValue (distance),
  //                              "DeltaY", DoubleValue (distance),
  //                              "GridWidth", UintegerValue (5),
  //                              "LayoutType", StringValue ("RowFirst"));
  // mobilityAdhoc.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  // mobilityAdhoc.Install (c);
  //***************************END********************************

//***************************DSR********************************
//   DsrHelper dsr;
//   DsrMainHelper dsrMain;

  InternetStackHelper internet;
  internet.Install (c);
//  dsrMain.Install (dsr, c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);
//***************************END********************************
//***************************working****************************
for (size_t i = 1; i < c.GetN(); i++) {
     ObjectFactory m_agentFactory;
     m_agentFactory.SetTypeId ("ns3::dsr::DsrRouting");
     Ptr<ns3::dsr::DsrRouting> agent = m_agentFactory.Create<ns3::dsr::DsrRouting> ();
     // deal with the downtargets, install UdpL4Protocol, TcpL4Protocol, Icmpv4L4Protocol
     Ptr<UdpL4Protocol> udp = c.Get (i)->GetObject<UdpL4Protocol> ();
     agent->SetDownTarget (udp->GetDownTarget ());
     udp->SetDownTarget (MakeCallback (&dsr::DsrRouting::Send, agent));
     Ptr<TcpL4Protocol> tcp = c.Get (i)->GetObject<TcpL4Protocol> ();
     tcp->SetDownTarget (MakeCallback (&dsr::DsrRouting::Send, agent));
     Ptr<Icmpv4L4Protocol> icmp = c.Get (i)->GetObject<Icmpv4L4Protocol> ();
     icmp->SetDownTarget (MakeCallback (&dsr::DsrRouting::Send, agent));
     c.Get (i)->AggregateObject (agent);
     //agent->SetRoute (Ipv4Address ("10.0.0.7"),Ipv4Address ("10.0.0.9"));
     agent->SetNode (c.Get (i));
}
ObjectFactory m_agentFactory;
  m_agentFactory.SetTypeId ("ns3::dsr::DsrRouting");
   Ptr<ns3::dsr::DsrRouting> agent = m_agentFactory.Create<ns3::dsr::DsrRouting> ();
   // deal with the downtargets, install UdpL4Protocol, TcpL4Protocol, Icmpv4L4Protocol
   Ptr<UdpL4Protocol> udp = c.Get (0)->GetObject<UdpL4Protocol> ();
   agent->SetDownTarget (udp->GetDownTarget ());
   udp->SetDownTarget (MakeCallback (&dsr::DsrRouting::Send, agent));
   Ptr<TcpL4Protocol> tcp = c.Get (0)->GetObject<TcpL4Protocol> ();
   tcp->SetDownTarget (MakeCallback (&dsr::DsrRouting::Send, agent));
   Ptr<Icmpv4L4Protocol> icmp = c.Get (0)->GetObject<Icmpv4L4Protocol> ();
   icmp->SetDownTarget (MakeCallback (&dsr::DsrRouting::Send, agent));
   c.Get (0)->AggregateObject (agent);

   Ptr<ns3::dsr::DsrRouteCache> routeCache= CreateObject<ns3::dsr::DsrRouteCache> ();


   // dsr::DsrOptionRrepHeader h;
   //
   // std::vector<Ipv4Address> nodeList;
   // nodeList.push_back (Ipv4Address ("1.1.1.0"));
   // nodeList.push_back (Ipv4Address ("1.1.1.1"));
   // nodeList.push_back (Ipv4Address ("1.1.1.2"));
   // h.SetNodesAddress (nodeList);
//   NS_TEST_EXPECT_MSG_EQ (h.GetNodeAddress (0), Ipv4Address ("1.1.1.0"), "trivial");
//   NS_TEST_EXPECT_MSG_EQ (h.GetNodeAddress (1), Ipv4Address ("1.1.1.1"), "trivial");
//   NS_TEST_EXPECT_MSG_EQ (h.GetNodeAddress (2), Ipv4Address ("1.1.1.2"), "trivial");


   //uint32_t bytes = p->RemoveHeader (h2);
  // std::vector<Ipv4Address> m_finalRoute;
   // m_finalRoute.push_back(i.GetAddress (4));
   // m_finalRoute.push_back(i.GetAddress (5));
   // m_finalRoute.push_back(i.GetAddress (6));

// DsrRouteCacheEntry::DsrRouteCacheEntry toSource (m_finalRoute,i.GetAddress (6),interPacketInterval);
//bool isRouteInCache = agent->LookupRoute (i.GetAddress (5),toPrev);
//std::cout << "success: "<<std::boolalpha<< isRouteInCache<<endl;
//DsrRouteCacheEntry::IP_VECTOR ip = toPrev.GetVector (); // The route from our own route cache to dst
//PrintVector (ip);
//std::vector<Ipv4Address> saveRoute (nodeList);
//PrintVector (saveRoute);
//bool areThereDuplicates = IfDuplicates (ip,saveRoute);
    // std::map<Ipv4Address, routeEntryVector> m_sortedRoutes;
// bool AddRoute (DsrRouteCacheEntry & rt);
//DsrRouteCacheEntry::DsrRouteCacheEntry (IP_VECTOR const  & ip, Ipv4Address dst, Time exp)

//bool flag1 =routeCache->AddRoute(toPrev);
//agent->SetRoute (Ipv4Address ("10.0.0.7"),Ipv4Address ("10.0.0.9"));
bool flag=routeCache->UpdateRouteEntry(i.GetAddress (5));
std::cout << "success: "<<std::boolalpha<< flag << " " << '\n';

agent->SetNode (c.Get (0));
//  Ptr<ns3::dsr::RouteCache> routeCache = CreateObject<ns3::dsr::RouteCache> ();
//  Ptr<ns3::dsr::RreqTable> rreqTable = CreateObject<ns3::dsr::RreqTable> ();
 //dsr->SetRouteCache (routeCache);
//  dsr->SetRequestTable (rreqTable);
//dsrSpecific->SetNode (c.Get (1));
//  node->AggregateObject (routeCache);
//  node->AggregateObject (rreqTable);
//***************************socket********************************
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (sinkNode), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (c.Get (sourceNode), tid);
  InetSocketAddress remote = InetSocketAddress (i.GetAddress (sinkNode, 0), 80);
  source->Connect (remote);
//***************************END***********************************

  // Give DSR time to converge-- 30 seconds perhaps
  Simulator::Schedule (Seconds (30.0), &GenerateTraffic,
                       source, packetSize, numPackets, interPacketInterval);

  // Output what we are doing
  NS_LOG_UNCOND ("Testing from node " << sourceNode << " to " << sinkNode << " with grid distance " << distance);

  Ptr<FlowMonitor> flowmon;
  FlowMonitorHelper flowmonHelper;
  flowmon = flowmonHelper.InstallAll ();

  Simulator::Stop (Seconds (33.0));
  AnimationInterface anim ("Secure-DSR-in-MANET/dsr-output.xml");
  Simulator::Run ();
  flowmon->SetAttribute("DelayBinWidth", DoubleValue(0.01));
  flowmon->SetAttribute("JitterBinWidth", DoubleValue(0.01));
  flowmon->SetAttribute("PacketSizeBinWidth", DoubleValue(1));
  flowmon->CheckForLostPackets();
  flowmon->SerializeToXmlFile("scratch/dsr-flow.xml", true, true);
  Simulator::Destroy ();

  return 0;
}
