/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */ /**
 * Copyright (c) 2011-2015 Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/
#include "ns3/core-module.h" 
#include "ns3/network-module.h" 
#include "ns3/point-to-point-module.h" 
#include "ns3/point-to-point-layout-module.h" 
#include "ns3/ndnSIM-module.h" 
#include "ns3/node-container.h" 
#include "ns3/ndnSIM/apps/bigdata-strategy.hpp" 
#include "ns3/ndnSIM/apps/bigdata-default-strategy.hpp" 
#include "ns3/ndnSIM/helper/ndn-link-control-helper.hpp" 
#include <iostream> 
#include <random> 
#include <algorithm> 
#include <chrono>
namespace ns3 { 


 //implement function for node distribution(Admin, Storage, User)
 bool contains(std::vector<int>& vec, int value){
    bool res = true;
     if(std::find(vec.begin(), vec.end(), value) != vec.end()) {
        /* vec contains value */
            res = true;
        } else {
            res = false;
        }
    return res ;
}
int main(int argc, char* argv[]) { 

        uint32_t dimension = 10;
	uint32_t nbCompute = 8;
	uint32_t nbSegments = 2;
        uint32_t exec = 2;
	uint32_t cs = 100;
	uint32_t mtbf = 100;
	uint32_t mfd = 100;
	uint32_t nbClient = 1;
	uint32_t nbFailure = 10;
	uint32_t speed =10;
	uint32_t delay = 1;
	string policy="Fifo";
	uint32_t runNumber = 1;

	// Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
	CommandLine cmd;
	cmd.AddValue ("dimension", "Grid dimension", dimension);
	cmd.AddValue ("nbCompute", "Number of ComputeNode", nbCompute);
	cmd.AddValue ("segments", "Number of segments per data file", nbSegments);
	cmd.AddValue ("exec", "Code execution time", exec);
	cmd.AddValue ("cs", "Content Store size", cs);
	cmd.AddValue ("mtbf", "MeanTime BetweenFailure", mtbf);
	cmd.AddValue ("mfd", "MeanFailureDuration", mfd);
	cmd.AddValue ("nbClient", "Number of client nodes", nbClient);
	cmd.AddValue ("nbFailure", "Number of failed nodes", nbFailure);
	cmd.AddValue ("speed", "Speed of the links", speed);
	cmd.AddValue ("delay", "Delay over the links", delay);
        cmd.AddValue ("policy", "Cache replacement policy", policy);
	cmd.AddValue ("runNumber", "Run number", runNumber);
	cmd.Parse(argc, argv);



        //set the dumpfile location;
    
      static GlobalValue g_myGlobal =
        GlobalValue ("myGlobal", "Value for the dump file",
               StringValue (std::string("../../bigdata/dump-trace-")
         +std::to_string(runNumber)+std::string(".txt")),
               MakeStringChecker ());

    //set file header
    ns3::StringValue stringValue;
    ns3::GlobalValue::GetValueByName ("myGlobal", stringValue);

    string file = stringValue.Get ();
    std::ofstream outfile;
    outfile.open(file, std::ios_base::app);
    outfile << "Time;Node;Type;Name;Hop" << std::endl;
    outfile.close();
    

    //the execution time is constant for the code

                static GlobalValue g_myExec =
                        GlobalValue ("myGlobalExec", "Value for the code execution time",
               StringValue (std::to_string(exec)),
               MakeStringChecker ());
        ns3::StringValue stringVal;
        ns3::GlobalValue::GetValueByName ("myGlobalExec", stringVal);


            
	  const int max = (const int) dimension;
          std::string speedValue = std::to_string(speed)+std::string("Mbps");
          std::string delayValue = std::to_string(delay)+std::string("ms");

             // Setting default parameters for PointToPoint links and channels
          Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue(speedValue));
          Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue(delayValue));


	 // Setting default parameters for PointToPoint links and channels
 	// Creating x*x topology
	PointToPointHelper p2p;
	PointToPointGridHelper grid(max, max, p2p);
	grid.BoundingBox(100, 100, 200, 200);
	NodeContainer nodes;
	for (int i = 0; i < max; ++i) {
		for (int j = 0; j < max; ++j) {
			nodes.Add(grid.GetNode(i, j));
		}
	}

      cout << "start random node selection " << endl; 

      unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
      std::default_random_engine generator (seed);
      std::uniform_int_distribution<int> distribution(0,dimension*dimension-1);

          //node repartition
          std::vector<int> idCompute(nbCompute);
          std::vector<int> idClient(nbClient);

          std::vector<int> idCodeData(2);

          //Compute nodes
               for (uint32_t j=0; j<nbCompute; ++j) {
                int number = distribution(generator);
                 if(contains(idCompute,number)){
                    j--;
                }else{
                    idCompute.at(j) = number;
                }
              }

        //Client nodes
               for (uint32_t i=0; i<nbClient; ++i) {
                int number = distribution(generator);
                 if(contains(idClient,number)||contains(idCompute,number)){
                    i--;
                }else{
                    idClient.at(i) = number;
                }
              }
        //Code and Data nodes
               for (uint32_t i=0; i<idCodeData.size(); ++i) {
                int number = distribution(generator);
                 if(contains(idClient,number)||contains(idCompute,number)||contains(idCodeData,number)){
                    i--;
                }else{
                    idCodeData.at(i) = number;
                }
              }
      cout << "end random node selection " << endl; 

	// Install NDN stack on all nodes
	ndn::StackHelper ndnHelper;
	
        ndnHelper.SetOldContentStore("ns3::ndn::cs::" + policy, "MaxSize",
                std::to_string(cs));
	ndnHelper.InstallAll();
	ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
	ndnGlobalRoutingHelper.InstallAll();
 
        std::vector<ApplicationContainer> container; //prepare for node failure simulation;
        
        cout << "start application installation on nodes " << endl;
	//Install client application on ClientNode

        double stime = 5.0; //use to specify the start time for the admin
        ndn::AppHelper clientHelper("ns3::ndn::Client");
        for (std::vector<int>::iterator it=idClient.begin(); it != idClient.end(); ++it){

                std::string prefCom = std::string("/lacl")+std::to_string(*it)+std::string("/compute");
                std::string prefData = std::string("/lacl")+std::to_string(*it)+std::string("/data");
                std::string prefCode = std::string("/lacl")+std::to_string(*it)+std::string("/code");
                ndn::StrategyChoiceHelper::InstallAll(prefCom, "/localhost/nfd/strategy/bigdata"); //install bigdata Strategy for all the storage stuff
                ndn::StrategyChoiceHelper::InstallAll(prefData, "/localhost/nfd/strategy/bigdatadefault"); //use the customized default which is best route + dump stuff
                ndn::StrategyChoiceHelper::InstallAll(prefCode, "/localhost/nfd/strategy/bigdatadefault");

                clientHelper.SetAttribute("PrefixCommand",StringValue(prefCom));
                clientHelper.SetAttribute("PrefixData",StringValue(prefData));
                clientHelper.SetAttribute("PrefixCode",StringValue(prefCode));
                clientHelper.SetAttribute("LastSegment",StringValue(std::to_string(nbSegments)));
                ApplicationContainer app = clientHelper.Install(nodes.Get(*it));
                app.Start(Seconds(stime));
                container.push_back(app);
                stime = stime + 10.0;
        }
        //install compute nodes application
        ndn::AppHelper computeHelper("ns3::ndn::Compute");
        for (std::vector<int>::iterator it=idCompute.begin(); it != idCompute.end(); ++it){
              for (std::vector<int>::iterator i=idClient.begin(); i != idClient.end(); ++i){

                  std::string prefCom= std::string("/lacl")+std::to_string(*i)+std::string("/compute");
                  computeHelper.SetAttribute("PrefixCommand", StringValue(prefCom));
                  ApplicationContainer app = computeHelper.Install(nodes.Get(*it));
                  container.push_back(app);
              }
          }


        //install code and data nodes applications 
    
        int number = idCodeData.at(0);

        for (std::vector<int>::iterator it=idClient.begin(); it != idClient.end(); ++it){

            std::string prefCom= std::string("/lacl")+std::to_string(*it)+std::string("/code");
            computeHelper.SetAttribute("PrefixCommand", StringValue(prefCom));
            //ApplicationContainer app = 
            computeHelper.Install(nodes.Get(number));
            //container.push_back(app);
        }

        number = idCodeData.at(1); 

        for (std::vector<int>::iterator it=idClient.begin(); it != idClient.end(); ++it){

            std::string prefCom= std::string("/lacl")+std::to_string(*it)+std::string("/data");
            computeHelper.SetAttribute("PrefixCommand", StringValue(prefCom));
            //ApplicationContainer app = 
            computeHelper.Install(nodes.Get(number));
            //container.push_back(app);
        }
        cout << "end application installation on nodes " << endl;

cout << "ContainerNumber of application :" << container.size()   << endl; 
 
/*   
    //Simulate node failure
    std::uniform_int_distribution<int> distrib(0,container.size()-1);
    std::vector<int> failed(nbFailure); //Keep track of already failed ...
    std::poisson_distribution<> timeFailureDistribution(mtbf); //for the failure;
    std::poisson_distribution<> failureDistribution(mfd);
    for(uint32_t i=0; i<nbFailure; i++){
        int failureTime = timeFailureDistribution(generator);
        int failureDuration = failureDistribution(generator);
        int element = distrib(generator);
cout << "Will fail :" << element << endl; 
        container.at(element).Stop(Seconds(failureTime)); //failure
        container.at(element).Start(Seconds(failureTime+failureDuration)); //restore
        
    }
*/

      cout << "Ready to start :" << endl;

      ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes();


       Simulator::Stop(Seconds(1000.0));

       ndn::L3RateTracer::InstallAll(std::string("../../bigdata/rate-trace-")
        +std::to_string(runNumber)+std::string(".txt"), Seconds(0.5));
  
       ndn::CsTracer::InstallAll(std::string("../../bigdata/cs-trace-")
        +std::to_string(runNumber)+std::string(".txt"), Seconds(0.5)); // 

	Simulator::Run();
	Simulator::Destroy();
	return 0;
}
} // namespace ns3
int main(int argc, char* argv[]) {
	return ns3::main(argc, argv);
}
