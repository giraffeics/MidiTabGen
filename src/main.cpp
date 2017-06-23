#include <iostream>
#include <sstream>
#include <MidiFile.h>

using namespace std;

int highestString(int);
string toLower(string str);
float getBurden(int, int, int, int, int);

int numStrings = 6;

int tuning[] = {
    52, 47, 43, 38, 33, 28
};

string noteNames[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

struct tab
{
    uint8_t str;
    uint8_t fret;
};

tab* tabs = nullptr;
int numNotes = 0;

int offset = -24;

int handPos = 1;

int main(int argc, char** argv)
{
    stringstream* lines = new stringstream[numStrings];

    string ifname = ""; //read in file name from argument 1
    if(argc < 2)
    {
        cout << "Drag a midi file from Windows Explorer into this window and press the enter key." << endl;
        getline(cin, ifname);

        int length = ifname.size();

        int f = 0;
        int l = length;

        if(ifname[0] == '"')
        {
            f = 1;
            l -= 1;
        }
        if(ifname[length-1] == '"')
            l -= 1;

        ifname = ifname.substr(f, l);
        length -= 2;
    }
    else
        ifname = argv[1];

    if(toLower(ifname.substr(ifname.size()-4, 4)) != ".mid")
    {
        cerr << "The file provided is not a midi file!" << endl;
        exit(1);
    }
    string ofname = ifname.substr(0, ifname.size()-4) + ".txt";

    MidiFile infile;
    infile.read(ifname);

    if (!infile.status())
    {
      cerr << "Error reading MIDI file " << ifname << endl;
      exit(1);
    }

    int track = infile.getTrackCount();
    if(track>1)
    {
        cout << "This file contains " << track << " tracks, numbered 0 through " << (track-1) << ". Which would you like to use?" << endl;
        cin >> track;
    }
    else
        track = 0;

    cout << "How many octaves do you want to shift this track up or down by?" << endl;
    cin >> offset;
    offset *= 12;

    ofstream outfile;
    outfile.open(ofname, ios::out);

    for(int i=0; i<infile[track].size(); i++)   //calculate number of notes
        if(infile[track][i].isNoteOn())
            numNotes++;

    tabs = new tab[numNotes];

    for(int i=0,n=-1; i<infile[track].size(); i++) //first pass in tab construction
    {
        if(!infile[track][i].isNoteOn())
        {
            continue;
        }

        n++;

        int pitch = (int)infile[track][i][1] + offset;
        tabs[n].str = highestString(pitch);
        tabs[n].fret = pitch - tuning[tabs[n].str];

        float minBurden;
        if(n==0)
            minBurden = 0;
        else
            minBurden = getBurden(handPos, tabs[n-1].str, tabs[n-1].fret, tabs[n].str, tabs[n].fret);

        for(int j=0; j<numStrings; j++)
        {
            if(tuning[j] < pitch)
            {
                float burden = getBurden(handPos, tabs[n-1].str, tabs[n-1].fret, j, pitch-tuning[j]);
                if(burden < minBurden)
                {
                    tabs[n].str = j;
                    tabs[n].fret = pitch-tuning[j];
                    minBurden = burden;
                }
            }
        }

        if(tabs[n].fret < handPos-1)
            handPos = tabs[n].fret;
        if(tabs[n].fret > handPos+4)
            handPos = tabs[n].fret-3;

        tabs[n].fret = pitch - tuning[tabs[n].str];
    }

    for(int i=0; i<numNotes; i++)   //construct strings of tabs for each string
    {
        lines[tabs[i].str] << (int)tabs[i].fret << '-';

        for(int j=0; j<numStrings; j++)
        {
            if(j != tabs[i].str)
            {
                lines[j] << "--";
            }
        }
    }

    for(int i=0; i<numStrings; i++)     //print tabs
    {
        outfile << noteNames[tuning[i]%12] << "-|" << lines[i].str() << endl;
    }

    outfile.close();

    cout << "Successfully created tabs." << endl;

    return 0;
}

float getBurden(int hand1, int str1, int fret1, int str2, int fret2)
{
    float burden = str1 - str2; //calculate burden based on switching strings
    if(burden<0)
        burden*=-1;
    burden -= 0.5f;
    if(burden<0)
        burden=0;

    if(fret2 < hand1)   //calculate burden based on slight stretch
        burden += 0.5;
    if(fret2 > hand1 + 3)
        burden += 0.5;

    if(fret1 - fret2 == 4 || fret2 - fret1 == 4)    //calculate burden based on double stretch
        burden += 0.5f;

    if(fret2 < hand1 - 1)   //calculate burden based on full hand movement
        burden += hand1 - fret2;
    if(fret2 > hand1 + 4)
        burden += fret2 - hand1 + 3;

    return burden;
}

int highestString(int note) //highest string that can play a given note
{
    int str = 0;
    int min_fret = INT_MAX;

    for(int i=0; i<numStrings; i++)
    {
        if(tuning[i] <= note && note - tuning[i] < min_fret)
        {
            str = i;
            min_fret = note - tuning[i];
        }
    }

    return str;
}

string toLower(string str)  //converts a string to lower case
{
    string ret = str;

    int l = ret.size();

    for(int i=0; i<l; i++)
    {
        ret[i] = tolower(ret[i]);
    }

    return ret;
}
