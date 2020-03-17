#include <iostream>
 #include "ns3/core-module.h"
 #include "ns3/network-module.h"
 #include "ns3/internet-module.h"
 #include "ns3/mobility-module.h"
 #include "ns3/dsr-module.h"
 #include "ns3/applications-module.h"
 #include "ns3/yans-wifi-helper.h"
#include "ns3/netanim-module.h"

 using namespace ns3;
 using namespace dsr;

 NS_LOG_COMPONENT_DEFINE ("manet-routing-compare");

 class RoutingExperiment
 {
 public:
   RoutingExperiment (); //constructor
   void Run ();

 private:
   Ptr<Socket> SetupPacketReceive (Ipv4Address addr, Ptr<Node> node);
   void ReceivePacket (Ptr<Socket> socket);

   uint32_t port;
   uint32_t bytesTotal;
   uint32_t packetsReceived;
 };

 RoutingExperiment::RoutingExperiment ()
   : port (9),
     bytesTotal (0),
     packetsReceived (0)
 {
 }

 void RoutingExperiment::ReceivePacket (Ptr<Socket> socket)
 {
   Ptr<Packet> packet;
   Address senderAddress;
   while ((packet = socket->RecvFrom (senderAddress)))
     {
       bytesTotal += packet->GetSize ();
       packetsReceived += 1;
     }
std::cout<<bytesTotal;
 }


 Ptr<Socket> RoutingExperiment::SetupPacketReceive (Ipv4Address addr, Ptr<Node> node)
 {
   TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

   Ptr<Socket> sink = Socket::CreateSocket (node, tid);
   InetSocketAddress local = InetSocketAddress (addr, port);
   sink->Bind (local);
   sink->SetRecvCallback (MakeCallback (&RoutingExperiment::ReceivePacket, this));

   return sink;
 }


 void RoutingExperiment::Run ()
 {

   int nWifis = 14;

   double TotalTime = 200.0;
   std::string rate ("2048bps");
   std::string phyMode ("DsssRate11Mbps");


   Config::SetDefault  ("ns3::OnOffApplication::PacketSize",StringValue ("64"));
   Config::SetDefault ("ns3::OnOffApplication::DataRate",  StringValue (rate));


   NodeContainer nodes;
   nodes.Create (nWifis);

   // setting up wifi phy and channel using helpers
   WifiHelper wifi;
   wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

   YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
   YansWifiChannelHelper wifiChannel;
   wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
   wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
   wifiPhy.SetChannel (wifiChannel.Create ());

   // Add a mac and disable rate control
   WifiMacHelper wifiMac;
   wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                 "DataMode",StringValue (phyMode),
                                 "ControlMode",StringValue (phyMode));

   wifiMac.SetType ("ns3::AdhocWifiMac");
   NetDeviceContainer adhocDevices = wifi.Install (wifiPhy, wifiMac, nodes);




   MobilityHelper mobilityAdhoc;
   mobilityAdhoc.SetPositionAllocator ("ns3::GridPositionAllocator",
                                "MinX", DoubleValue (0.0),
                                "MinY", DoubleValue (0.0),
                                "DeltaX", DoubleValue (500),
                                "DeltaY", DoubleValue (500),
                                "GridWidth", UintegerValue (5),
                                "LayoutType", StringValue ("RowFirst"));
   mobilityAdhoc.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
   mobilityAdhoc.Install (nodes);
int jump=1,jump1=0,jump2=1;
   for (uint n=0 ; n < nodes.GetN() ; n++)
    {
      if(n<4){
       Ptr<ConstantVelocityMobilityModel> mob = nodes.Get(n)->GetObject<ConstantVelocityMobilityModel>();
       mob->SetVelocity(Vector(0, 0, 0));
       mob->SetPosition(Vector(jump, 1.15, 0));
       jump++;

     }
     if(n>=4&&n<9){
      Ptr<ConstantVelocityMobilityModel> mob = nodes.Get(n)->GetObject<ConstantVelocityMobilityModel>();
      mob->SetVelocity(Vector(0, 0, 0));
      mob->SetPosition(Vector(jump1, 2.3, 0));
         jump1+=2;
    }
    if(n>=9){
     Ptr<ConstantVelocityMobilityModel> mob = nodes.Get(n)->GetObject<ConstantVelocityMobilityModel>();
     mob->SetVelocity(Vector(0, 0, 0));
     mob->SetPosition(Vector(jump2, 3.3, 0));
      jump2++;
   }
    }


   DsrHelper dsr;
   DsrMainHelper dsrMain;
   InternetStackHelper internet;

       internet.Install (nodes);
       dsrMain.Install (dsr, nodes);

   NS_LOG_INFO ("assigning ip address");

   Ipv4AddressHelper addressAdhoc;
   addressAdhoc.SetBase ("10.1.1.0", "255.255.255.0");
   Ipv4InterfaceContainer adhocInterfaces;
   adhocInterfaces = addressAdhoc.Assign (adhocDevices);

   OnOffHelper onoff1 ("ns3::UdpSocketFactory",Address ());
   onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
   onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));


    for (int i = 0; i < 3 ; i++)
      {

       Ptr<Socket> sink = SetupPacketReceive (adhocInterfaces.GetAddress (i), nodes.Get (i));
  std::cout<<sink;
       AddressValue remoteAddress (InetSocketAddress (adhocInterfaces.GetAddress (i), port));
       onoff1.SetAttribute ("Remote", remoteAddress);

       Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
       ApplicationContainer temp = onoff1.Install (nodes.Get (i));
       temp.Start (Seconds (var->GetValue (0.0,1.0)));
       temp.Stop (Seconds (TotalTime));
     }

   Simulator::Stop (Seconds (TotalTime));
   AnimationInterface anim("anim1.xml");
   Simulator::Run ();
   Simulator::Destroy ();
 }



 int main (int argc, char *argv[])
 {
   RoutingExperiment experiment;
   experiment.Run ();
 }
