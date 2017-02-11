/**
    PA 1, CS 130A
    Prof. Coakley
    By Hanqing Wang
*/
#include <iostream>
#include <time.h>
#include <cstdlib>
using namespace std;
#define TIMEOUT 8640000000l
#define ATTACK_EVENT 1
#define FIX_EVENT 2
#define NOTIFY_EVENT 3
#define LATENCY 100l
#define HACKER -1
#define ATTACK_FREQ 1000l
#define FIX_FREQ 10000l
#define DEFAULT_HEAP_SIZE 20
/*
    For events occuring at the SAME TIME, Attack events have higher priority than Fix events, which have higher priority than Notify events.
    This means that Attack events are popped first from the event queue in case there is a tie.
    The operator overloads for the Node class show this.
*/
/*
    Event class
    parameter id:
    1=AttackEvent
    2=FixEvent
    3=NotifyEvent
*/
struct Node{
    long long eventTime;
    int id;
    int source;
    int target;
    Node(){}
    Node(long long eventTime,int id, int source, int target):eventTime(eventTime),id(id),source(source),target(target){}
    bool operator<(Node node){
        return (this->eventTime!=node.eventTime && this->eventTime<node.eventTime) ||(this->eventTime==node.eventTime && this->id<node.id);
    }
    bool operator<=(Node node){
        return ((*this)<node)||(this->eventTime==node.eventTime && this->id==node.id);
    }
    bool operator>(Node node){
        return (this->eventTime!=node.eventTime && this->eventTime>node.eventTime) ||(this->eventTime==node.eventTime && this->id>node.id);
    }
    bool operator>=(Node node){
        return ((*this)>node)||(this->eventTime==node.eventTime && this->id==node.id);
    }
    friend ostream& operator<<(ostream& o,const Node&node){//Determines what type of event this node is, then prints out appropriately
        if(node.id==ATTACK_EVENT){
            o<<"Attack("<<node.eventTime<<", ";
            if(node.source!=HACKER)
                o<<node.source;
            else
                o<<"H";
            o<<", "<<node.target<<")";
        }else if(node.id==FIX_EVENT){
            o<<"Fix("<<node.eventTime<<", "<<node.target<<")";
        }else if(node.id==NOTIFY_EVENT){
            o<<"Notify("<<node.eventTime<<", ";
            if(node.source!=HACKER)
                o<<node.source;
            else
                o<<"H";
            o<<", "<<node.target<<")";
        }else{
            o<<"Event("<<node.eventTime<<")";
        }
    }
};

/**
    Binary Heap/Priority Queue
*/
class Heap{
    public:
        Heap(){
            heapSize=0;
            capacity=DEFAULT_HEAP_SIZE;
            events=new Node[DEFAULT_HEAP_SIZE];
        }
        ~Heap(){
            delete[]events;
        }
        void push(Node node){
            if(isFull()){
                doubleAllocatedMemory();
            }
            events[heapSize]=node;
            int currentIndex=heapSize;
            while(!isRootIndex(currentIndex)){
                int parentIndex=getParentIndex(currentIndex);
                if(events[parentIndex]>events[currentIndex]){
                    Node temp(events[parentIndex]);
                    events[parentIndex]=events[currentIndex];
                    events[currentIndex]=temp;
                    currentIndex=parentIndex;
                }else{
                    break;
                }
            }
            heapSize++;
        }
        Node pop(){
            if(isEmpty()){
                throw runtime_error("Heap is empty.");
            }
            Node root=events[0];
            events[0]=events[heapSize-1];
            heapSize--;
            int currentIndex=0;
            while(currentIndex<heapSize){
                int leftChildIndex=getLeftChildIndex(currentIndex);
                int rightChildIndex=getRightChildIndex(currentIndex);
                int smallerChildIndex=0;
                if(leftChildIndex<heapSize){
                    if(rightChildIndex<heapSize)
                        smallerChildIndex=(events[leftChildIndex]<=events[rightChildIndex])?leftChildIndex:rightChildIndex;
                    else
                        smallerChildIndex=leftChildIndex;
                }else if(rightChildIndex<heapSize)
                    smallerChildIndex=rightChildIndex;
                else break;
                if(events[currentIndex]<=events[smallerChildIndex]) break;
                Node temp(events[currentIndex]);
                events[currentIndex]=events[smallerChildIndex];
                events[smallerChildIndex]=temp;
                currentIndex=smallerChildIndex;
            }

            return root;
        }
        int size(){return heapSize;}
        bool isFull(){return heapSize==capacity;}
        bool isEmpty(){return heapSize==0;}
        friend ostream& operator<<(ostream& o,const Heap&heap){
            o<<"[";
            for(int i=0;i<heap.heapSize;i++){
                o<<heap.events[i];
                if(i!=heap.heapSize-1)o<<", ";
            }
            o<<"]";
        }
    private:
        Node*events;
        int heapSize;
        int capacity;
        void doubleAllocatedMemory(){
            Node*temp=new Node[capacity*2];
            for(int i=0;i<capacity;i++){
                temp[i]=events[i];
            }
            delete[]events;
            events=temp;
            capacity<<=1;
        }
        bool isRootIndex(int index){return index==0;}
        int getParentIndex(int index){return (index-1)/2;}
        int getLeftChildIndex(int index){return (index+1)*2-1;}
        int getRightChildIndex(int index){return (index+1)*2;}
};

/*
    Notes: Only successful attacks are printed.
    -1 represents the HACKER, so Node(5000,ATTACK_EVENT,-1,2) means that the hacker is attacking computer 2.
    Nodes represent events and different types of events, see Node classes.
*/
void queueAttack(Heap&h,long long atkTime, int source, int numComputers){
    int nextTarget=0;
    while((nextTarget=rand()%numComputers)==source){}//Reroll when attacker chooses itself until a different character is chosen
    h.push(Node(atkTime,ATTACK_EVENT,source,nextTarget));
}
void queueNotify(Heap&h,long long eventTime, int source, int target){
    h.push(Node(eventTime,NOTIFY_EVENT,source,target));
}
void queueFix(Heap&h,long long eventTime,int target){
    h.push(Node(eventTime,FIX_EVENT,0,target));
}
int main(int argc, char *argv[])
{
    if(argc!=4){
	cout<<"Incorrect number of arguments, should be in the form of ./program_name number_computers attack_success detect_success, \
i.e. ./program_name 1000 4 100.";
	return 0;
    }
    srand (time(NULL));
    Heap h;
    int numComputers=atoi(argv[1]);
    int percentAttackSuccess=atoi(argv[2]);
    int percentDetect=atoi(argv[3]);
    int numInfected=0;
    bool*computerInfected=new bool[numComputers];
    for(int i=0;i<numComputers;i++)computerInfected[i]=false;
    int startOfSecondBranch=(numComputers+1)/2;//Example: numComputers=59, then left half=0..29, right half=30..58
    queueAttack(h,0l,HACKER,numComputers);
    int result=0;
    bool timeout=false;
    while(numInfected<=numComputers/2&&!h.isEmpty()){
        Node temp=h.pop();
        if(temp.eventTime>TIMEOUT){
            timeout=true;
            break;
        }
        if(temp.id!=1)cout<<temp<<endl;//Don't print out failed attacks
        if(temp.id==1){
            bool attackSuccess=rand()%100<percentAttackSuccess;//random number between 0 and 99, compare it to attack success percent
            long long nextAttackTime=temp.eventTime+ATTACK_FREQ;
            if(attackSuccess){
                cout<<temp<<endl;
                if(!computerInfected[temp.target])numInfected++;
                computerInfected[temp.target]=true;
                if((temp.source==HACKER||(temp.source<startOfSecondBranch&&temp.target>=startOfSecondBranch)||
                   (temp.source>=startOfSecondBranch&&temp.target<startOfSecondBranch))&&rand()%100<percentDetect){
                        queueNotify(h,LATENCY+temp.eventTime,temp.source,temp.target);
                }
                queueAttack(h,LATENCY+nextAttackTime,temp.target,numComputers);
            }
            queueAttack(h,nextAttackTime,temp.source,numComputers);
        }else if(temp.id==2){
            if(computerInfected[temp.target])numInfected--;
            computerInfected[temp.target]=false;
            if(numInfected==0)break;
        }else if(temp.id==3){
            if(temp.source!=HACKER)
                queueFix(h,temp.eventTime+LATENCY+FIX_FREQ,temp.source);
            queueFix(h,temp.eventTime+LATENCY+2l*FIX_FREQ,temp.target);//Target cannot be hacker.
        }
    }
    if(numInfected>numComputers/2&&!timeout){
        cout<<"HACKER WINS"<<endl;
    }else if(numInfected==0&&!timeout){
        cout<<"COMPUTER WINS"<<endl;
    }else cout<<"DRAW"<<endl;
    delete[]computerInfected;
    return 0;
}
/*
    SOME HEAP TEST CODE


    Heap h;
    h.push(Node(655,1,32,52));
    h.push(Node(17,2,322,43));
    h.push(Node(23,3,43,432));
    h.push(Node(72,1,123,322332));
    h.push(Node(32,3,23,53));
    h.push(Node(14,1,23,2));
    cout<<h<<endl;
    h.pop();
    h.pop();
    cout<<h<<endl;
    h.push(Node(33,1,223,23));
    h.push(Node(34,2,32,2));
    h.push(Node(34,1,3,5));
    h.pop();
    cout<<h<<endl;
    for(int i=0;i<7555555;i++){
        h.push(Node(i*10,i%3+1,3,4));
    }
    for(int i=0;i<7555555;i++){
        h.pop();
    }
    cout<<h;

    MORE TEST CODE (for breaking ties)
    Heap h;
    h.push(Node(23,3,2,3));
    h.push(Node(23,2,3,432));
    h.push(Node(23,1,432,32));
    cout<<h<<endl;
*/
