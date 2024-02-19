#include "HybridDetect.h"
#include <windows.h>
#include <TlHelp32.h>
#include <cstdio>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <iostream>
#include <tchar.h>
#include <atlstr.h>
#include <WtsApi32.h>
#include <tchar.h>
#include <psapi.h>
#include "pdh.h"

using namespace std;

vector<int> coreMapArr;

void coreParitionDetermination() {
    // determine work load and if needed to shift to group of cores 
    // every x (5) second check current CPU utilization on both ECores and Pcores
    Sleep(50000);
}

DWORD findAndBind(const wchar_t* target, int selectedAffinity) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    BOOL found = FALSE;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE) {
        while (Process32Next(snapshot, &entry) == TRUE) {
            if (wcscmp(entry.szExeFile, target) == 0) {

                HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, TRUE, entry.th32ProcessID);

                DWORD_PTR processAffinityMask = selectedAffinity;
                BOOL success = SetProcessAffinityMask(hProcess, processAffinityMask);
                SetPriorityClass(hProcess, PROCESS_MODE_BACKGROUND_END);
                if (success == TRUE) {
                    cout << success + " Bind was successful" << endl;
                    found = TRUE;
                    //system("pause");
                }
                else {
                    cout << " ERROR -- Retry bind" << endl;
                    system("pause");
                }
                CloseHandle(hProcess);
            }
        }
    }
    else {
        cout << "ERROR -- #" << endl;
        system("pause");
    }
    if (found == FALSE) {
        cout << "ERROR -- Program is not currenlty running" << endl;
        system("pause");
    }
    cout << "\n" << endl;
    CloseHandle(snapshot);
    return 0;
}

const char* bool_cast(const bool b) {
    return b ? "TRUE" : "FALSE";
}

int affinityMaskGenerator(int coreCount) {
    int affintyMask = 0;

    for (int i = 0; i < coreCount; i++) {
        affintyMask += pow(2, i);
    }
    return affintyMask;
}

int chooseAffinity() {
    system("cls");
    string programTarget;
    cout << ("\nEnter program target: ");
    cin >> programTarget;
    string fullProgramTrgt = programTarget + ".exe";

    wstring widestring = wstring(fullProgramTrgt.begin(), fullProgramTrgt.end());
    const wchar_t* targetPrgm = widestring.c_str();

    int selectedAffinity = 0;

    for (auto i : coreMapArr) {
        cout << "Option #: " << i << ' ' << endl;
    }
    cout << "\n" << endl;

    do {
        cout << "Type affinity choice: ";
        cin >> selectedAffinity;
        if (cin.fail())
        {
            cout << "ERROR -- You did not enter an integer";

            cin.clear();
#undef max
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    } while (selectedAffinity > coreMapArr.size() || selectedAffinity < 0);
    selectedAffinity--;
    findAndBind(targetPrgm, coreMapArr[selectedAffinity]);
    return 0;
}

int customCoreSelect(int logicalCores, int eCores, int pCores) {
    system("cls");
    coreMapArr.clear();
    cout << " - Custom mode selected - " << endl;
    int coreType = 0, eCoreUser = 0, pCoreUser = 0, affinityOffset = 0;
    int hThreading = 1;
    int hyperthreadCores = logicalCores - eCores;

    cout << ("1. [P Cores]\n");
    cout << ("2. [E Cores]\n");
    cout << ("3. [Hybrid]\n");
    //change to case statment
    do {
        cout << "\nSelect core type: ";
        cin >> coreType;
        if (cin.fail())
        {
            cout << "ERROR -- You did not enter an integer";

            // get rid of failure state
            cin.clear();
#undef max
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    } while ((coreType < 1) && (coreType > 3));

    //Custom pC
    if (coreType == 1) {

        //enable or disable hThreading
        //set switch case statement
        do {
            cout << "\nEnabled(1) or Disable(0) hyperthreading: ";
            cin >> hThreading;
            if (cin.fail())
            {
                cout << "ERROR -- You did not enter an integer" << endl;
                // get rid of failure state
                cin.clear();
#undef max
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        } while (hThreading > 1 || hThreading < 0);

        string boolean = bool_cast(hThreading);
        cout << "Hyperthreading set - " + boolean << endl;
        if (hThreading == 1) {
            do {
                cout << "P-Cores avaliable: " << hyperthreadCores;
                cout << "\nDefine max pCores: ";
                cin >> pCoreUser;


                if (cin.fail())
                {
                    cout << "ERROR -- You did not enter an integer";
                    // get rid of failure state
                    cin.clear();
#undef max
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                }
                //Only pick ints even and within parameter ranegs of Max and 0
            } while (((pCoreUser % 2) == 1) || ((pCoreUser <= 0) || (pCoreUser > hyperthreadCores)));

            // adds functionality but needs stepping functionality
            int hThreadAffinity = affinityMaskGenerator(pCoreUser);
            coreMapArr.push_back(hThreadAffinity);

        }
        else if (hThreading == 0) {
            do {
                cout << "-- BROKEN [WIP] -- " << endl;
                cout << "\nP-Cores avaliable: " << pCores;
                cout << "\nDefine max pCores: ";
                cin >> pCoreUser;
                if (cin.fail())
                {
                    cout << "ERROR -- You did not enter an integer";
                    // get rid of failure state
                    cin.clear();
#undef max
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }

                //introduce stepping function...
            } while ((pCoreUser <= 0) || (pCoreUser > pCores));
        }
        else {
            cout << "ERROR -- Program Abort..." << endl;
            system("pause");
            abort();
        }

    }
    //Custom eCores
    else if (coreType == 2) {
        do {
            cout << "\nE-Cores avaliable: " << eCores;
            cout << "\nDefine max eCores: ";
            cin >> eCoreUser;
            if (cin.fail())
            {
                cout << "ERROR -- You did not enter an integer";
                // get rid of failure state 

                cin.clear();
#undef max
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        } while ((eCoreUser <= 0) || (eCoreUser > eCores));

        int coreOffset = eCoreUser + hyperthreadCores;

        affinityOffset = affinityMaskGenerator(hyperthreadCores);
        int tmpAffinity = affinityMaskGenerator(coreOffset);
        int eCoreAffinity = tmpAffinity - affinityOffset;
        //cout << eCoreAffinity << endl;
        coreMapArr.push_back(eCoreAffinity);
    }
    else if (coreType == 3) {

        do {
            cout << "P-Cores avaliable: " << hyperthreadCores;
            cout << "\nDefine max pCores: ";
            cin >> pCoreUser;


            if (cin.fail())
            {
                cout << "ERROR -- You did not enter an integer";
                // get rid of failure state
                cin.clear();
#undef max
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

            }
            //Only pick ints even and within parameter ranegs of Max and 0
        } while (((pCoreUser % 2) == 1) || ((pCoreUser <= 0) || (pCoreUser > hyperthreadCores)));

        int hThreadAffinity = affinityMaskGenerator(pCoreUser);

        do {
            cout << "\nE-Cores avaliable: " << eCores;
            cout << "\nDefine max eCores: ";
            cin >> eCoreUser;
            if (cin.fail())
            {
                cout << "ERROR -- You did not enter an integer";
                // get rid of failure state 

                cin.clear();
#undef max
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        } while ((eCoreUser <= 0) || (eCoreUser > eCores));

        int coreOffset = eCoreUser + hyperthreadCores;

        affinityOffset = affinityMaskGenerator(hyperthreadCores);
        int tmpAffinity = affinityMaskGenerator(coreOffset);
        int eCoreAffinity = tmpAffinity - affinityOffset;

        int hybridAffinity = eCoreAffinity + hThreadAffinity;
        coreMapArr.push_back(hybridAffinity);

    }
    else {
        cout << "ERROR -- Custom mode setup failed" << endl;
        system("pause");
    }
    return 0;
}

int generalCoreSelect(int eCores, int logicalCores) {
    coreMapArr.clear();
    int hyperTCores = logicalCores - eCores;

    //Set All_core affinity
    int allCoreAffinity = affinityMaskGenerator(logicalCores);
    coreMapArr.push_back(allCoreAffinity);

    //Set Hyperthreaded pCore affinity
    int pCoreAffinity = affinityMaskGenerator(hyperTCores);
    coreMapArr.push_back(pCoreAffinity);

    //Set eCore affinity
    int eCoreAffinity = allCoreAffinity - pCoreAffinity;
    coreMapArr.push_back(eCoreAffinity);

    return 0;
}

void SetProcessPriority(const wchar_t* ProcessName, int Priority) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    BOOL found = FALSE;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE) {
        while (Process32Next(snapshot, &entry) == TRUE) {
            if (wcscmp(entry.szExeFile, ProcessName) == 0) {

                HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, TRUE, entry.th32ProcessID);


                BOOL success = SetPriorityClass(hProcess, Priority);
                SetPriorityClass(hProcess, Priority);
                if (success == TRUE) {
                    cout << success + " Re-Prioritze was successful" << endl;
                    found = TRUE;
                    //system("pause");
                }
                else {
                    cout << " ERROR -- Retry Set-Priority" << endl;
                    system("pause");
                }
                CloseHandle(hProcess);
            }
        }
    }
    else {
        cout << "ERROR -- Interesting error..." << endl;
        system("pause");
    }
    if (found == FALSE) {
        cout << "ERROR -- Program is not currenlty running" << endl;
        system("pause");
    }
    cout << "\n" << endl;
    CloseHandle(snapshot);
}

int priorityMenu() {
    char char_choice[3];
    int int_choice = 0;
    string programName;
    cout << ("Enter program name: ");
    cin >> programName;
    string fullProgramName = programName + ".exe";

    wstring widestring = wstring(fullProgramName.begin(), fullProgramName.end());
    const wchar_t* tmp = widestring.c_str();

    do
    {
        system("cls");
        cout << ("\n");
        cout << ("Priority Selector: \n\n");
        cout << ("1. Low\n");
        cout << ("2. Below-Normal\n");
        cout << ("3. Normal\n");
        cout << ("4. Above-Normal\n");
        cout << ("5. High\n");
        cout << ("6. Extrene Realtime\n\n");
        cout << ("0. Back\n");

        cout << ("\nEnter choice: ");
        cin >> ("%s", char_choice);
        int_choice = atoi(char_choice);

        switch (int_choice)
        {
        case 1:
            //IDLE / Idle
            SetProcessPriority(tmp, 0x00000040);
            break;
        case 2:
            //B-Normal
            SetProcessPriority(tmp, 0x00004000);
            break;
        case 3:
            //Normal
            SetProcessPriority(tmp, 0x00000020);
            break;
        case 4:
            //A-Normal
            SetProcessPriority(tmp, 0x00008000);
            break;
        case 5:
            //High
            SetProcessPriority(tmp, 0x00000080);
            break;
        case 6:
            //Reat-time
            SetProcessPriority(tmp, 0x00000100);
            break;
        case 0:
            return 0;
            break;
        default:cout << ("Invalid Choice. Re-enter");
            break;
        }
    } while (int_choice != 99);
}

void processesSnapShot(int mask) {
    system("cls");

    //int mask = affinityMaskGenerator(coreSelected);

    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwPriorityClass;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        cout << "Error";
    }
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        cout << "Error loading first";
        CloseHandle(hProcessSnap);
    }
    do {
        cout << pe32.th32ProcessID << endl;
        HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, TRUE, pe32.th32ProcessID);

        DWORD_PTR processAffinityMask = mask;
        BOOL success = SetProcessAffinityMask(hProcess, mask);
        SetPriorityClass(hProcess, PROCESS_MODE_BACKGROUND_END);

    } while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    system("pause");
}


void menuSelect(int lCore, int eCore, int pCore) {
    char char_choice[3];
    int int_choice = 0;

    int coreOffset = eCore + lCore;
    int hyperthreadCores = lCore - eCore;

    int affinityOffset = affinityMaskGenerator(hyperthreadCores);
    int tmpAffinity = affinityMaskGenerator(lCore);
    int eCoreMask = tmpAffinity - affinityOffset;

    int pCoreMask = affinityMaskGenerator(hyperthreadCores);
    int lCoreMask = affinityMaskGenerator(lCore);
    do
    {
        system("cls");
        cout << ("\n");
        cout << ("\t\t\t -- Main Menu -- \n\n");
        cout << ("1. General Binds\n");
        cout << ("2. Create Custom Single Binds\n");
        cout << ("3. Create Custom All Binds\n\n");
        cout << ("4. Re-Prioritize Program\n");
        cout << ("\n5. Full P-Core Mode\n");
        cout << ("6. Full E-Core Mode\n");
        cout << ("7. Default Mode\n");
        cout << ("\n\n0. Exit\n");

        cout << ("\nEnter choice: ");
        cin >> ("%s", char_choice);
        int_choice = atoi(char_choice);

        switch (int_choice)
        {
        case 1:
            generalCoreSelect(eCore, lCore);
            chooseAffinity();
            break;
        case 2:
            customCoreSelect(lCore, eCore, pCore);
            chooseAffinity();
            break;
        case 3:
            // Sets a custom core select for all running programs.
            customCoreSelect(lCore, eCore, pCore);
            processesSnapShot(coreMapArr[0]);
            coreMapArr.clear();
            break;
        case 4:
            //sets every instance of Chrome to HIGH priority
            priorityMenu();
            break;
        case 5:
            // Pin every program to pcores.
            processesSnapShot(pCoreMask);
            break;
        case 6:
            // Pin every program to pcores
            processesSnapShot(eCoreMask);
            break;
        case 7:
            // clear Modes.
            processesSnapShot(lCoreMask);
            break;
        case 0:
            exit(1);
            break;
        default:cout << ("Invalid Choice. Re-enter");
            break;
        }
    } while (int_choice != 99);
}
int __cdecl main(int argc, char** argv) {
    int lCore_Count = 0;
    for (EnumLogicalProcessorInformation enumInfo(RelationGroup);
        auto pinfo = enumInfo.Current(); enumInfo.MoveNext()) {
        lCore_Count = pinfo->Group.GroupInfo->MaximumProcessorCount;
    }

    int pCore_count = 0, tCore_count = 0, eCore_count = 0;
    int i = 0;

    for (EnumLogicalProcessorInformation enumInfo(RelationProcessorCore);
        auto pinfo = enumInfo.Current(); enumInfo.MoveNext()) {
        for (UINT GroupIndex = 0; GroupIndex < pinfo->Processor.GroupCount; GroupIndex++) {

            if ((int)pinfo->Processor.EfficiencyClass == 1)
            {
                pCore_count++;
            }
            else {
                eCore_count++;
            }

        }

    }
    tCore_count = pCore_count + eCore_count;

    menuSelect(lCore_Count, eCore_count, pCore_count);

    return 0;
}