/**
 * @file fwdc.cpp
 * @author Steven Clark
 * @date 9/25/2017
 * @brief Source code for A* solver for Farmer Wolf Duck & Corn logic problem.
 */

#include <iostream>
#include <vector>
#include <string>
#include <map>

using std::string;
using std::vector;
using std::multimap;
using std::map;
using std::cout;
using std::endl;

/**
 * @brief Farmer Wolf Duck and Corn game state
 */
class FWDCstate{
public:
	bool FL;///<is farmer on left bank of the river
	bool WL;///<is wolf on left bank of the river
	bool DL;///<is duck on left bank of the river
	bool CL;///<is corn on left bank of the river

	///@brief Construct a new state with all items on the right bank of the river
	FWDCstate(){
		FL = 0;
		WL = 0;
		DL = 0;
		CL = 0;
	}

	///@brief Construct a new state with some items on the left bank of the river
	///@param FF Is the farmer on the left bank?
	///@param WW Is the wolf on the left bank?
	///@param DD is the duck on the left bank?
	///@param CC is the corn on the left bank?
	FWDCstate(bool FF, bool WW, bool DD, bool CC){
		FL=FF;
		WL=WW;
		DL=DD;
		CL=CC;
	}
	///@brief Less than operator for placement in ordered data structures
	///@note neccessary for use as an STL map or set key
	bool operator<(const FWDCstate &other) const{
		if(FL != other.FL){
			return other.FL;
		}else if(WL != other.WL){
			return other.WL;
		}else if(DL != other.DL){
			return other.DL;
		}else if(CL != other.CL){
			return other.CL;
		}else
			return false;
	}

	bool operator==(const FWDCstate &other) const{
		return FL == other.FL and WL == other.WL and DL == other.DL and CL == other.CL;
	}

	bool operator!=(const FWDCstate &other) const{
		return FL != other.FL or WL != other.WL or DL != other.DL or CL != other.CL;
	}

	bool isWinning()const{
		return FL and WL and DL and CL;
	}

	///@brief Computes a heuristic for all items to reach the left bank
	int h()const{
		return /*(int)!FL+*/(int)!WL+(int)!DL+(int)!CL;
	}

	///@brief Can the farmer and wolf be moved to the other bank next turn without creating an illegal state
	bool canMoveFW()const{
		return FL == WL and DL != CL;
	}
	///@brief Can the farmer and corn be moved to the other bank next turn without creating an illegal state
	bool canMoveFC()const{
		return FL == CL and WL != DL;
	}
	///@brief Can just the farmer be moved to the other bank next turn without creating an illegal state
	bool canMoveF()const{
		return DL != CL and WL != DL;
	}
	///@brief Can the farmer and duck be moved to the other bank next turn without creating an illegal state
	bool canMoveFD()const{
		return FL == DL;
	}
	///@brief Get all legal game states that can be expanded from this one
	///@return a vector of states one move from this one
	vector <FWDCstate> nextStates()const{
		vector <FWDCstate> rvec;
		if(canMoveFW())
			rvec.push_back(FWDCstate(!FL,!WL,DL,CL));
		if(canMoveFD())
			rvec.push_back(FWDCstate(!FL,WL,!DL,CL));
		if(canMoveFC())
			rvec.push_back(FWDCstate(!FL,WL,DL,!CL));
		if(canMoveF())
			rvec.push_back(FWDCstate(!FL,WL,DL,CL));

		return rvec;
	}
	///@brief Get a string representation of the problem state.
	string toString()const{
		string rval = "[";
		if(FL)
			rval.append("F");
		if(WL)
			rval.append("W");
		if(DL)
			rval.append("D");
		if(CL)
			rval.append("C");
		rval.append("||");
		if(!FL)
			rval.append("F");
		if(!WL)
			rval.append("W");
		if(!DL)
			rval.append("D");
		if(!CL)
			rval.append("C");
		rval.append("]");
		return rval;
	}
};

/**
 * @brief A fully generated problem space graph node with A* information
 */
struct PSNode {
	FWDCstate state;///<problem state itself
	PSNode * parent;///<parent node in the problem space graph if any
	int cost2reach;///<the number of moves taken to reach this node from the start, g()
	int projectedCost;///<the heuristic estimate number of moves to complete the problem, h()
	vector <PSNode *> children;///<the child nodes in the problem space graph

	///@brief New problem space graph node given problem state and parent node.
	PSNode(FWDCstate newstate, PSNode * from){
		state = newstate;
		parent = from;
		if(NULL == from)
			cost2reach = 0;
		else
			cost2reach = from->cost2reach + 1;
		projectedCost = state.h();
	}

	///@brief Update the cost to reach this node (and any children) if new cost is better.
	///@param newcost The cost of the new path to this node found.
	///@param newparent The node to backtrack along this new path.
	///@param frontier The frontier of the problem space graph to update  with a new f if neccesary
	///@return True if the new path was supperior on the path was updated
	bool updateCostCond(int newcost, PSNode * newparent, multimap<int, PSNode *> &frontier){
		if(newcost < cost2reach){
			//look for node in frontier
			for(multimap<int, PSNode*>::iterator iter = frontier.lower_bound(cost2reach); iter != frontier.upper_bound(cost2reach); iter++){
				if(iter->second == this){
					frontier.erase(iter);//if in frontier remove it and emplace with new adjusted cost
					frontier.insert(std::pair<int,PSNode *>(newcost+projectedCost,this));
					break;
				}
			}

			cost2reach = newcost;
			parent = newparent;

			//update any children
			for(unsigned int i = 0; i < children.size(); ++i){
				children[i]->updateCostCond(newcost + 1, this, frontier);
			}
			return true;
		}
		return false;
	}
};

//used to simplify template syntax
typedef std::pair<int,PSNode *> FrontierPair;
typedef std::pair<FWDCstate, PSNode *> GeneratedPair;

int main(int argc, char** argv){
	PSNode * winningNode = NULL;

	//map of all generated gamestates to their problem space graph nodes
	map <FWDCstate, PSNode *> generated;

	//map of all frontier nodes by their f() costs
	multimap <int, PSNode *> frontier;

	//node currently being evaluated, begins at problem start state;
	PSNode *tempNode = new PSNode(FWDCstate(),NULL);
	PSNode * workNode = NULL;//just a temp
	string outpath, tempstring;//temps for formatting output of the wining path

	//Add start state to generated nodes and frontier
	generated.insert(GeneratedPair(tempNode->state, tempNode));
	frontier.insert(FrontierPair(1,tempNode));

	//While we haven't won or lost
	while(winningNode == NULL and ! frontier.empty()){

		//output the current frontier nodes
		cout << "Frontier nodes are:\t";
		for(multimap<int, PSNode*>::iterator iter = frontier.begin();
				iter != frontier.end(); ++iter){
			cout << iter->second->state.toString()
					<< " h="<< iter->second->projectedCost
					<< " g="<< iter->second->cost2reach
					<< " f="<< iter->first << endl;
		}

		//chose the node with the lowest cost in the frontier
		tempNode = frontier.begin()->second;
		cout << "Expand:\t" << tempNode->state.toString() << endl;

		//expand it
		frontier.erase(frontier.begin());
		vector<FWDCstate> tempChildren = tempNode->state.nextStates();
		for(unsigned int i = 0; winningNode == NULL and i < tempChildren.size();++i){
			if(tempChildren[i] != tempNode->state){//if generated state not that of parent
				cout << "Generated:\t" << tempChildren[i].toString() << '\t';

				//if state in question is already generated, updated if neccesary
				if(generated.count(tempChildren[i]) > 0){
					cout << "Regenerated\t";
					if(generated[tempChildren[i]]->updateCostCond(tempNode->cost2reach+1, tempNode, frontier))
						cout << "Updated F\t";
					else
						cout << "No update\t";
				}else{//generate the graph node for this state
					cout << "New node\t        \t";
					workNode = new PSNode(tempChildren[i],tempNode);
					generated.insert(GeneratedPair(workNode->state,workNode));
					frontier.insert(FrontierPair(workNode->cost2reach + workNode->projectedCost,workNode));
					if(workNode->state.isWinning()){
						winningNode = workNode;
					}
				}

				//Add the node, generated or new to the children of tempNode
				tempNode->children.push_back(generated[tempChildren[i]]);

				cout << "g=" << generated[tempChildren[i]]->cost2reach
						<< " h=" << generated[tempChildren[i]]->projectedCost
						<< " f=" << generated[tempChildren[i]]->cost2reach + generated[tempChildren[i]]->projectedCost
						<< endl;
			}

		}
	}

	//If the winning path was found, print it
	if(winningNode != NULL){
		cout << "Winning state reached." << endl;
		for(workNode = winningNode; workNode != NULL; workNode = workNode->parent){
			outpath = workNode->state.toString() + outpath;
			if(workNode->parent != NULL)
				outpath = " -> " + outpath;
		}
		cout << outpath << endl;
	}else{
		cout << "No path to goal!" << endl;
	}

	//remove all nodes in the problem space graph from the heap
	for(map <FWDCstate, PSNode *>::iterator iter = generated.begin(); iter != generated.end(); ++iter){
		delete iter->second;
	}
	return 0;
}
