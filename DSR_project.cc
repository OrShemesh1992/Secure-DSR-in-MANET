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
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <math.h>
#include "ns3/dsdv-module.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/animation-interface.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"

using namespace ns3;
using namespace std;


NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhocGrid");

class Experiment{

//functions of the class
public:
std::string m_protocolName;
string toplogyName;
std::string m_CSVfileName;
map<int, int> packetsRecievedPerNode;
map<int, int> packetsLostPerNode;
map<int, double> RatioPerNode;
map<int, double> ThroughputPerNode;
map<int, double> BytesPerNode;
map<int, double> timePerPacket;
double sumAvg;
double delaySum;
int count_CheckThroughput;
double simulationTime;
double startSendingTime;
double interval;// seconds
double totalTimeToSendPackets;



Experiment (uint32_t protocol, uint32_t topology);
void ReceivePacket (Ptr<Socket> socket);
static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval );
void Run (std::string CSVfileName);
std::string CommandSetup (int argc, char **argv);
void CheckThroughput ();
void lines (NodeContainer c);
void circle (NodeContainer c);
void grid (NodeContainer c);
void locateOnCircle (NodeContainer c, std::set<pair<double,double>> points);
double circleEquasion(double x, double y, double center);
double randomDouble(double low, double high);

void square (NodeContainer c);
void random(NodeContainer c);
void randomWalk(NodeContainer c ,MobilityHelper mobilityAdhoc);
void setNames();
void setCSVFile();

//members of the class
private:
  std::string phyMode = "DsssRate1Mbps";
  double distance; // m
  double RX;
  uint32_t packetSize; // bytes
  double numPackets;
  uint32_t numNodes;  // by default, 5x5
  double start_send_packet;
  double time;
  int recvPackets;
  uint32_t m_protocol;
  int position;
  vector<int> destNodes;
  vector<int> sourceNodes;
  std::vector<Ptr<Socket>> recieves;
  std::vector<Ptr<Socket>> sources;
  string netAnimFileName;
  void CircleWithLine (NodeContainer c);

};

//initalize constructor
Experiment::Experiment (uint32_t protocol, uint32_t topology){
    simulationTime = 30.0;
    distance = 1000;  // m
    packetSize = 1000; // bytes
    numPackets = 60;
    numNodes = 50;  // by default, 5x5
    interval = 0.5; // seconds
    startSendingTime = 1.0;
    //m_CSVfileName = "Dsr_project.csv";
    recvPackets = 0;
    RX = -57;
    start_send_packet=0;
    time=0;
    count_CheckThroughput = 0;
    sumAvg = 0.0;
    totalTimeToSendPackets = 0.0;
    delaySum = 0.0;
    m_protocol = protocol; // 1-olsr, 2-aodv, 3-DSR
    position = topology; //1-lines, 2-circle, 3-grid, 4-square, 5 -random, 6 -circle
    destNodes = { 45, 48, 49};
    sourceNodes = {1,1,1};
    setNames();
    setCSVFile();
    netAnimFileName = "animations/" + m_protocolName + " " + toplogyName + ".xml";
}

//writing data to the csv file
void Experiment::setNames(){
  switch (position)
    {
    case 1:
      toplogyName = "lines";
      break;
    case 2:
      toplogyName = "circle";
      break;
    case 3:
      toplogyName = "grid";
      break;
    case 4:
      toplogyName = "square";
      break;
    case 5:
      toplogyName = "random";
      break;
    case 6:
      toplogyName = "CircleWithLine";
      break;
    case 7:
      toplogyName = "randomWalk";
      break;
    default:
      NS_FATAL_ERROR ("No such pos:" << position);
    }
    switch (m_protocol)
      {
        case 1:
          m_protocolName = "OLSR";
          break;
        case 2:
          m_protocolName = "AODV";
          break;
        case 3:
          m_protocolName = "DSR";
          break;
        default:
          NS_FATAL_ERROR ("No such protocol:" << m_protocol);
      }
}

void Experiment::setCSVFile(){
  string fileName = "statistics/"+ m_protocolName + " " + toplogyName+".csv";
  m_CSVfileName = fileName;
//writing the coloums to csv file
  std::ofstream out (fileName.c_str ());
  out << "seconds," <<
  "NumberOfNodes," <<
  "distance," ;
  for (auto i: destNodes) {
    out<< "PacketsForNode "<< i <<", ";
    out<< "Lost Packets "<< i <<", ";
  }
  out<<"Ratio,"<<"Number Of Packets per Node," <<
  "Recieved Packets," <<
  "Protocol," <<"Average time per packet,"<<
  std::endl;
  out.close ();

}
 void
 Experiment::CheckThroughput ()
 {
   count_CheckThroughput+=1;
   std::ofstream out (m_CSVfileName.c_str (), std::ios::app);
   double countRecieved=0,countLost=0;
   if(time==0){
     time=(Simulator::Now ()).GetSeconds ();
   }else{
     start_send_packet=time;
     time=(Simulator::Now ()).GetSeconds ();
   }
   out << time << ","
       << numNodes << ","
       << distance << ",";
       for (auto i: destNodes) {
         countRecieved+=packetsRecievedPerNode[i];
         countLost+=numPackets-packetsRecievedPerNode[i];
         packetsLostPerNode[i]=numPackets-packetsRecievedPerNode[i];
         out<< packetsRecievedPerNode[i] <<",";
         out<<numPackets-packetsRecievedPerNode[i]<<",";
       }

       for (auto i: destNodes) {
         RatioPerNode[i] = (packetsRecievedPerNode[i] / numPackets)*100;
         //std::cout << RatioPerNode[i] << '\n';
        }
        for (auto i : destNodes) {
        ThroughputPerNode[i] = (BytesPerNode[i]*8)/(simulationTime*1024*1024);
        //std::cout << ThroughputPerNode[i] << '\n';
        }

      double Ratio=(countRecieved/numPackets)*100;
      double avg_time = time - start_send_packet;
      sumAvg+=avg_time;
      out<< Ratio <<","<< numPackets << ","
      << recvPackets << ","
      << m_protocolName << ","
      << avg_time << ","
      << std::endl;
      out.close ();
   //packetsReceived = 0;
//   Simulator::Schedule (Seconds (1.0), &Experiment::CheckThroughput, this);
 }

//Receive packet
void Experiment:: ReceivePacket (Ptr<Socket> socket)
{
   while (socket->Recv ())
     {
       int node = socket->GetNode()->GetId();
       packetsRecievedPerNode[node]++;
       BytesPerNode[node]+=packetSize;
       double now = (Simulator::Now ()).GetSeconds ();
       double packetSendTime = (interval*(packetsRecievedPerNode[node]-1)) + startSendingTime;
       double delay = (now - packetSendTime);
       delaySum += delay;
       CheckThroughput();
       //std::cout <<node<< " received a packet" << '\n';
     }
}

//sending a packet
void Experiment:: GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {

      Ptr<Packet> p = Create<Packet> (pktSize);
      //std::cout << p->ToString()<<"check"<<std::endl;
      socket->Send (p);
      Simulator::Schedule (pktInterval, &GenerateTraffic,
                           socket, pktSize,pktCount - 1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}

std::string
Experiment::CommandSetup (int argc, char **argv)
{
  CommandLine cmd;
  cmd.AddValue ("CSVfileName", "The name of the CSV output file name", m_CSVfileName);
  //cmd.AddValue ("traceMobility", "Enable mobility tracing", m_traceMobility);
  //cmd.AddValue ("protocol", "1=OLSR;2=AODV;3=DSDV;4=DSR", m_protocol);
  cmd.Parse (argc, argv);
  return m_CSVfileName;
}


void Experiment::lines (NodeContainer c){
  double jump=0.5,jump1=0.2,jump2=0.5;
  uint size = (c.GetN())/3;
  for (uint n=0 ; n < c.GetN() ; n++){
     if(n < size){
      Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
      mob->SetVelocity(Vector(0,0,0));
      mob->SetPosition(Vector(jump, 1.15, 1));
      jump++;

    }
    if(n >=size && n<= size*2){
     Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
     mob->SetVelocity(Vector(0,0,0));
     mob->SetPosition(Vector(jump1, 3.45, 1));
        jump1++;
   }
   if(n > size*2){
    Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
    mob->SetVelocity(Vector(0,0,0));
    mob->SetPosition(Vector(jump2, 5.75, 1));
    jump2++;
   }
  }
}

void Experiment::circle (NodeContainer c) {
  // Circle equasion: (x-a)**2 + (y-b)**2 = R**2
  int size = c.GetN();
  double pi = 2*asin(1.0), center = 5;
  double radius = center;
  std::set<pair<double,double>> points;
  double x = 0, y = radius;
  pair <double, double> p(x,y);
  points.insert(p);
  double alpha = 360/size;
  double angle = alpha;
  for (size_t i = 1; i < c.GetN(); i++)
  {
    x = radius - (radius*cos((angle*pi)/180));
    y = radius + (radius*sin((angle*pi)/180));
    points.insert(make_pair(x,y));
    angle+=alpha;
  }
  int pointsSize = points.size();
  if(pointsSize == size){
    locateOnCircle(c, points);
  }
}

double Experiment::circleEquasion(double x, double y, double center){
  double res = (x-center)*(x-center)+(y-center)*(y-center);
  return res;
}

void Experiment::locateOnCircle (NodeContainer c, std::set<pair<double,double>> points){
  int ind = 0;
  for (auto i: points) {
    Ptr<ConstantVelocityMobilityModel> m = c.Get(ind)->GetObject<ConstantVelocityMobilityModel>();
    m->SetVelocity(Vector(0,0,0));
    m->SetPosition(Vector(i.first,i.second, 1));
    ind++;
  }
}


double Experiment::randomDouble(double low, double high){
  int range=(high-low)+1;
  return range * (rand() / (RAND_MAX + 1.0));
}

void Experiment::CircleWithLine (NodeContainer c) {
// Circle equasion: (x-a)**2 + (y-b)**2 = R**2
  RX = -63;
  int size = c.GetN(), lineNodes = size/4, circumferenceSize = (size-lineNodes);
  double pi = 2*asin(1.0), center = 5;
  double radius = center;
  std::set<pair<double,double>> points;
  double x = 0, y = radius;
  pair <double, double> p(x,y);
  points.insert(p);
  double alpha = 360/circumferenceSize;
  double angle = alpha;
  double gap = (2*pi*radius*alpha)/360;
//  double limitAngle = 160;
  // && angle < limitAngle
  while(angle<360) {
    x = radius - (radius*cos((angle*pi)/180));
    y = radius + (radius*sin((angle*pi)/180));
    points.insert(make_pair(x,y));
    angle+=alpha;
  }
  pair <double, double> p1(make_pair( (radius*2) - gap,radius) );
  points.insert(p1);
  points.insert(make_pair( gap,radius));
  double lineGap = lineNodes/(radius*2);
  y = center;
  x = lineGap;
  int pointsSize = points.size();
  while(pointsSize<size){
    points.insert(make_pair(x,y));
    x+=lineGap;
    pointsSize = points.size();
  }
  if(pointsSize == size){
    locateOnCircle(c, points);
  }
}
void Experiment::grid (NodeContainer c){
  for (uint n=0 ; n < c.GetN() ; n++)
   {
     Ptr<ConstantVelocityMobilityModel> p = c.Get(n)->GetObject<ConstantVelocityMobilityModel> ();
   }
}

void Experiment::square (NodeContainer c){
  double jump=0.5,jump1=0.5,jump2=0.5, jump3=0.5;
  uint size = c.GetN()/4;
  //std::cout<<size<<std::endl;
  for (uint n=0 ; n < c.GetN() ; n++)
   {
     //left vertical edge
     if(n < size){
      Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
      mob->SetVelocity(Vector(0,0,0));
      mob->SetPosition(Vector(0, jump, 1));
      jump+=1.5;
    }
    //right vertical edge
    if(n >=size && n< size*2){
     Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
     mob->SetVelocity(Vector(0,0,0));
     mob->SetPosition(Vector(jump, jump1, 1));
        jump1+=1.5;
   }
   //lower horizental edge
   if(n >= size*2 && n<size*3){
    Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
    mob->SetVelocity(Vector(0,0,0));
    mob->SetPosition(Vector(jump2, jump1, 1));
     jump2+=1.5;
  }
  //upper horizental edge
   if(n >= size*3) {
    Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
    mob->SetVelocity(Vector(0,0,0));
    mob->SetPosition(Vector(jump3, 0, 1));
     jump3+=1.5;
   }
 }
}

void Experiment::random(NodeContainer c){
  int low = 0, high = 10;
  std::set<pair<double,double>> points;
  while(true){
    double x = randomDouble(low,high), y = randomDouble(low,high);
    pair <double, double> p(x,y);
    points.insert(p);
  //  int pointsSize = points.size();
    if(points.size() == c.GetN()) break;
  }
  int ind = 0;
  for (auto i: points) {
    Ptr<ConstantVelocityMobilityModel> mob = c.Get(ind)->GetObject<ConstantVelocityMobilityModel>();
    mob->SetVelocity(Vector(0,0,0));
    mob->SetPosition(Vector(i.first, i.second, 1));
    ind++;
  }
}
void Experiment::randomWalk(NodeContainer c ,MobilityHelper mobilityAdhoc){
    mobilityAdhoc.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               "Bounds", RectangleValue (Rectangle (-1, 100, -1, 100)));
    mobilityAdhoc.Install (c);
}
void
Experiment::Run (std::string CSVfileName)
{

  Packet::EnablePrinting ();
  m_CSVfileName = CSVfileName;

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



//***************************SetPosition*************************


MobilityHelper mobilityAdhoc;
if (position!=7){
mobilityAdhoc.SetPositionAllocator ("ns3::GridPositionAllocator",
                             "MinX", DoubleValue (0.0),
                             "MinY", DoubleValue (0.0),
                             "DeltaX", DoubleValue (2),
                             "DeltaY", DoubleValue (2),
                             "GridWidth", UintegerValue (numNodes/5),
                             "LayoutType", StringValue ("RowFirst"));

mobilityAdhoc.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");

mobilityAdhoc.Install (c);
}
switch (position)
  {
  case 1:
    lines(c);
    std::cout<< "This is lines topology"<<std::endl;
    break;
  case 2:
    circle(c);
    std::cout<< "This is circle topology"<<std::endl;
    break;
  case 3:
    grid(c);
    std::cout<< "This is grid topology"<<std::endl;
    break;
  case 4:
    square (c);
    std::cout<< "This is square topology"<<std::endl;
    break;
  case 5:
    random (c);
    std::cout<< "This is random topology"<<std::endl;
    break;
  case 6:
    CircleWithLine(c);
    std::cout<< "This is circle with line topology"<<std::endl;
    break;
  case 7:
    randomWalk(c,mobilityAdhoc);
    std::cout<< "This is random Walk topology"<<std::endl;
    break;
  default:
    NS_FATAL_ERROR ("No such pos:" << position);
  }

  //***************************END********************************
  //***************************wifi*************************
    // The below set of helpers will help us to put together the wifi NICs we want
    WifiHelper wifi;

    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
    // set it to zero; otherwise, gain will be added
    wifiPhy.Set ("RxGain", DoubleValue (RX) );
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

//***************************DSR********************************

   AodvHelper aodv;
   OlsrHelper olsr;
   DsrHelper dsr;
   DsrMainHelper dsrMain;
   Ipv4ListRoutingHelper list;
   InternetStackHelper internet;

   switch (m_protocol)
     {
     case 1:
       list.Add (olsr, 100);
       m_protocolName = "OLSR";
       break;
     case 2:
       list.Add (aodv, 100);
       m_protocolName = "AODV";
       break;
     case 3:
       m_protocolName = "DSR";
       break;
     default:
       NS_FATAL_ERROR ("No such protocol:" << m_protocol);
     }
     std::cout<< "This is "<<m_protocolName<<" protocol"<<std::endl;
   if (m_protocol < 3)
     {
       internet.SetRoutingHelper (list);
       internet.Install (c);
     }
   else if (m_protocol == 3)
     {
       internet.Install (c);
       dsrMain.Install (dsr, c);
     }



  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ip = ipv4.Assign (devices);

///********************************try - see if works!!!
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

  std::vector<InetSocketAddress> remotes;
    InetSocketAddress remote=InetSocketAddress (Ipv4Address::GetAny (), 80);
  for (size_t i = 0; i < destNodes.size(); i++) {
    Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (destNodes[i]), tid);
    recvSink->Bind (local);
    recvSink->SetRecvCallback (MakeCallback (&Experiment::ReceivePacket, this));
    recieves.push_back(recvSink);
    remote = InetSocketAddress (ip.GetAddress (destNodes[i], 0), 80);
    Ptr<Socket> source = Socket::CreateSocket (c.Get (sourceNodes[i]), tid);
    source->Connect (remote);
    remotes.push_back(remote);
    sources.push_back(source);
    Simulator::Schedule (Seconds (startSendingTime), &GenerateTraffic,
                         sources[i], packetSize, numPackets, interPacketInterval);
    // Give DSR time to converge-- 30 seconds perhaps

  }
//***************************END of try code***********************************


  // Output what we are doing
//  NS_LOG_UNCOND ("Testing from node " << sourceNode << " to " << destNode << " with grid distance " << distance);

  Ptr<FlowMonitor> flowmon;
  FlowMonitorHelper flowmonHelper;
  flowmon = flowmonHelper.InstallAll ();

//  CheckThroughput();

  Simulator::Stop (Seconds (simulationTime));
  AnimationInterface anim (netAnimFileName);
  Simulator::Run ();
  for (auto itr = Experiment::packetsRecievedPerNode.begin(); itr != Experiment::packetsRecievedPerNode.end(); ++itr) {
          cout <<  "Number of packet received for node " << itr->first
               << " is: " << itr->second << '\n';
    }
  int sendingNodesSize = sourceNodes.size();
  totalTimeToSendPackets /= (double) sendingNodesSize;
  delaySum /= count_CheckThroughput;
  time = (Simulator::Now ()).GetSeconds ();
  totalTimeToSendPackets += (time - startSendingTime);

  flowmon->SetAttribute("DelayBinWidth", DoubleValue(0.01));
  flowmon->SetAttribute("JitterBinWidth", DoubleValue(0.01));
  flowmon->SetAttribute("PacketSizeBinWidth", DoubleValue(1));
  flowmon->CheckForLostPackets();
  flowmon->SerializeToXmlFile("scratch/dsr-flow.xml", true, true);
  Simulator::Destroy ();
}


int main (int argc, char *argv[])
{
  map<int,int> packetsNumber;
  map<int,int> PacketsLostNumber;
  map<int,int> RatioNumber;
  map<int,double> Throughput;
  string resultsSummary = "statistics/results Summary.csv";
  std::ofstream out (resultsSummary.c_str ());
  out <<"Protocol," <<
  "Topology, " <<
  "Average Packets Recieved, "<<
  "Average Packets Lost, "<<
  "Average Ratio, "<<
  "Average Time Per Packet, "<<
  "Throughput, "<<
  "End To End Delay, "<<
  "Average time to send all packets, "<<
   std::endl;
  string protocolName, topologyName;
  int sumOfPackets = 0;
  int sumOfLostPackets = 0;
  int sumOfRatio = 0;
  double sumOfThroughput = 0.0;
  double sumAverage = 0.0;
  double totalPacketNumber = 0.0;
  double delaySum =0;
  double timeToSendAllPackets = 0;
  for (size_t i = 1; i <= 3; i++) { // 1-olsr, 2-aodv, 3-DSR
    for (size_t j = 1; j <= 7; j++) {   //1-lines, 2-circle, 3-grid, 4-square, 5 -random, 6-CircleWithLine , 7-randomWalk
      size_t k = 0;
      for (k = 0; k < 5; k++) {
        Experiment experiment(i,j);
        protocolName = experiment.m_protocolName;
        topologyName = experiment.toplogyName;
        string fileName =  "statistics/"+ experiment.m_protocolName + " " + experiment.toplogyName+".csv";
        experiment.Run (fileName);
        //counting num of packets per run for each node
        for(auto itr = experiment.packetsRecievedPerNode.begin() ; itr!= experiment.packetsRecievedPerNode.end(); itr++){
          packetsNumber[itr->first]+=itr->second;
        }
        //counting num of lost packets per run for each node
        for(auto itr = experiment.packetsLostPerNode.begin() ; itr!= experiment.packetsLostPerNode.end(); itr++){
          PacketsLostNumber[itr->first]+=itr->second;
        }
        //counting ratio per run for each node
        for(auto itr = experiment.RatioPerNode.begin() ; itr!= experiment.RatioPerNode.end(); itr++){
          RatioNumber[itr->first]+=itr->second;
        }
        for(auto itr = experiment.ThroughputPerNode.begin() ; itr!= experiment.ThroughputPerNode.end(); itr++){
          Throughput[itr->first]+=itr->second;
        }
        sumAverage = experiment.sumAvg;
        totalPacketNumber = experiment.count_CheckThroughput;
        delaySum += experiment.delaySum;
        timeToSendAllPackets+= experiment.totalTimeToSendPackets;
      }
   //making average packets per node and summing the average to calculate general average for all nodes
     for(auto itr = packetsNumber.begin() ; itr!= packetsNumber.end(); itr++){
       int sum = itr->second;
       sum = sum/k;
       sumOfPackets+=sum;
       itr->second = 0;
     }
     //making average of lost packets per node and summing the average to calculate general average for all nodes
     for(auto itr = PacketsLostNumber.begin() ; itr!= PacketsLostNumber.end(); itr++){
       int sum = itr->second;
       sum = sum/k;
       sumOfLostPackets+=sum;
       itr->second = 0;
     }
     //making average of ratio per node and summing the average to calculate general average for all nodes
     for(auto itr = RatioNumber.begin() ; itr!= RatioNumber.end(); itr++){
       int sum = itr->second;
       sum = sum/k;
       sumOfRatio+=sum;
       itr->second = 0;
     }

     for(auto itr = Throughput.begin() ; itr!= Throughput.end(); itr++){
       double sum = itr->second;
       sum = sum/k;
       sumOfThroughput+=sum;
       itr->second = 0;
     }

     int averagePacketsRecieved = sumOfPackets/packetsNumber.size();
     int averagePacketsLost = sumOfLostPackets/PacketsLostNumber.size();
     int averageRatio = sumOfRatio/RatioNumber.size();
     double averageThroughput = sumOfThroughput/Throughput.size();
     double endToEndDelay = delaySum/(double)k;
     double avgTimeToSendAllPackets = timeToSendAllPackets/(double)k;


     //writing to CSV file
     out <<protocolName << ", " <<
     topologyName << ", "<<
     averagePacketsRecieved << ", "<<
     averagePacketsLost<< ", "<<
     averageRatio<< "%" <<", "<<
     (sumAverage/totalPacketNumber)/k<<", "<<
     averageThroughput<<", "<<
     endToEndDelay<<", "<<
     avgTimeToSendAllPackets<<", "<<
      std::endl;
      sumOfPackets = 0;
      sumOfLostPackets = 0;
      sumOfRatio = 0;
      averagePacketsRecieved = 0;
      averagePacketsLost = 0;
      averageRatio = 0;
      sumOfThroughput = 0.0;
      endToEndDelay = 0;
      delaySum = 0;
      avgTimeToSendAllPackets=0;
      timeToSendAllPackets=0.0;
    }
  }
  out.close ();
}
