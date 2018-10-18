#pragma region Libaries
#include <vector>
#include <iostream>
#include <string>
#include <stdio.h>
#include <ctime> 
#include <vector>
#include <cstdlib>
#include <math.h>
#include <map>
#include <iterator>
#include <math.h>
#include <iomanip>
#include <algorithm>
#include <string>
#include <stdlib.h>>
#include <windows.h>

#include <chrono>
#include <mutex>
std::mutex ilock;

using namespace std;
using namespace std::chrono;
#pragma endregion

#pragma region Functions Declarations

#pragma region UserInput

int userInput_CrafterLevel();
// @return int - an integer, user inputs their Crafter Level

int userInput_Craftsmanship();
// @return int - an integer, user inputs their craftsmanship

int userInput_Control();
// @return int - an integer, user inputs their control

int userInput_CP();
// @return int - an integer, user inputs their CP

int userInput_RecipeLevel();
// @return int - an integer, user inputs a RecipeLevel

int userInput_RecipeItemLevel();
// @return int - an integer, user inputs RecipeItemLevel

int userInput_ProgressRequired();
// @return int - an integer, user inputs ProgressRequired

int userInput_QualityRequired();
// @return int - an integer, user inputs qualityRequired

int userInput_RecipeDurability();
// @return int - an integer, user inputs recipeDurability

#pragma endregion

int GetCrafterLevelToItemLevelequivalent(int);
// Used to help figure out the Level difference Buff/Penalty (Not under effect of ING1 or ING2)

int GetCrafterLevelToItemLevelequivalentING1(int);
// Used to help figure out the Level difference Buff/Penalty while under the effect of ING1

int GetCrafterLevelToItemLevelequivalentING2(int);
// Used to help figure out the Level difference Buff/Penalty while under the effect of ING2

float calculateBaseProgressIncrease(int, int, int, int);
// Used to determine the level Corrected Progress 

int** GetQualityValues(float, float, int, int, int, int, float, float, int);
//Determines Quality Action Values

void GetQualityValuesVec(vector<vector<int> > &, float, float, int, float, float, float, float);
//Determines Quality Action Values

void printTableHeader(string);
// prints the Quality Type Name & Header formatting

void printTableHeader2(string, string);
// prints the Quality Type Name & Header formatting

void printVector(vector<vector<int> >&, int, int, int);
// prints the Quality Values of a particular Quality Action

void printVector2(vector<vector<int> >&, vector<vector<int> >&, int, int, int);
// prints the Quality Values of a particular Quality Action

void GetProgressValuesVec(vector<int> &, float, float, float, float);

void printProgressBuffs(float, float, float);

void printProgressValues(string, vector<int> &, float, float, float);

#pragma endregion

#pragma region Main
int main()
{
#pragma region System
	system("Title Crafting Sequence Solver");
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#pragma endregion

#pragma region UserInput_Variables

#pragma region Ask_variables
	//cout << "Crafter Details" << endl << endl;

	//int CrafterLevel = userInput_CrafterLevel();
	//int Craftsmanship = userInput_Craftsmanship();
	//int Control = userInput_Control();
	//int CP = userInput_CP();

	//cout << endl << endl << "Recipe Details" << endl << endl;

	//int RecipeLevel = userInput_RecipeLevel();
	//int RecipeItemLevel = userInput_RecipeItemLevel();
	//int Recipedifficulty = userInput_ProgressRequired();
	//int RecipeQualityRequired = userInput_QualityRequired();
	//int maxdurability = userInput_RecipeDurability();

	//cout << endl << endl;

#pragma endregion

#pragma region Auto_variables
	// Crafter Details
	int const CrafterLevel = 69;
	int const Craftsmanship = 1702;
	int const Control = 1731;
	int CP = 493 + 15;
	bool Specialist = 1;

	// Recipe Details
	int const RecipeLevel = 70;
	int const RecipeItemLevel = 290;
	int Recipedifficulty = 2952;
	int RecipeQualityRequired = 13187;
	int maxdurability = 80;
	bool HQ = 0;
	int durability = maxdurability;

#pragma endregion

#pragma region SolverVariables

	vector<string> ProgressNames;

	float PbP = 0.33;
	int currentProgress = 0;
	int countProgress = 0;

	// Determine if an action is available to use by the solver
	bool MuscleMemoryAvailable = 0;
	bool PiecebyPieceAvailable = 1;
	bool Ingenuity2Available = 0;
	bool Ingenuity1Available = 1;

	// keep track of action buffs
	int SHbuff = 0;
	int ING2buff = 0;
	int ING1buff = 0;
	int i = 0;

#pragma region ProgressSkillMultipliers
	//  Progress skill multipliers
	// E.g. a progress skill with 150% efficency has  a multiplier of 1.5
	float BasicSynthesisMultiplier = 1.0;
	float CarefulSynthesis1Multiplier = 0.9;
	float CarefulSynthesis2Multiplier = 1.2;
	float CarefulSynthesis3Multiplier = 1.5;
	float FocusedSynthesisMultiplier = 2.0;
#pragma endregion


#pragma endregion

#pragma endregion

#pragma region StartSequentialTime
	// Record start time
	auto startsequential = std::chrono::high_resolution_clock::now();
#pragma endregion

#pragma region CrafterToRecipeLevelDifferences

	// Figure out Crafter recipe item level equivilent while unbuffed, using ingenuity 1 & using ingenuity 2
	// Used to find the how much level difference there is between the crafter & recipe being crafted
	int crafterLevelToItemLevelequivalent = GetCrafterLevelToItemLevelequivalent(CrafterLevel);
	int crafterLevelToItemLevelequivalentING1 = GetCrafterLevelToItemLevelequivalentING1(RecipeItemLevel);
	int crafterLevelToItemLevelequivalentING2 = GetCrafterLevelToItemLevelequivalentING2(RecipeItemLevel);

	// Calculate level difference while unbuffed, using ingenuity 1 & using ingenuity 2
	float LevelDifference = crafterLevelToItemLevelequivalent - RecipeItemLevel;
	float LevelDifferenceING1 = crafterLevelToItemLevelequivalent - crafterLevelToItemLevelequivalentING1;
	float LevelDifferenceING2 = crafterLevelToItemLevelequivalent - crafterLevelToItemLevelequivalentING2;

	// Determine the base progress value after applying level difference & progress penalties for unbuffed, ingenuity 1, ingenuity 2
	float levelCorrectedProgress = calculateBaseProgressIncrease(LevelDifference, Craftsmanship, crafterLevelToItemLevelequivalent, RecipeItemLevel);
	float levelCorrectedProgressING1 = calculateBaseProgressIncrease(LevelDifferenceING1, Craftsmanship, crafterLevelToItemLevelequivalent, RecipeItemLevel);
	float levelCorrectedProgressING2 = calculateBaseProgressIncrease(LevelDifferenceING2, Craftsmanship, crafterLevelToItemLevelequivalent, RecipeItemLevel);

	if (LevelDifference > 0)
	{
		LevelDifference = 0;
	}
	if (LevelDifference < -10)
	{
		LevelDifference = -10;
	}

	if (LevelDifferenceING1 > 0)
	{
		LevelDifferenceING1 = 0;
	}
	if (LevelDifferenceING1 < -10)
	{
		LevelDifferenceING1 = -10;
	}

	if (LevelDifferenceING2 > 0)
	{
		LevelDifferenceING2 = 0;
	}
	if (LevelDifferenceING2 < -10)
	{
		LevelDifferenceING2 = -10;
	}

	LevelDifference = (1 + (0.05*LevelDifference));
	LevelDifferenceING1 = (1 + (0.05*LevelDifferenceING1));
	LevelDifferenceING2 = (1 + (0.05*LevelDifferenceING2));

	cout << endl;

#pragma endregion

	//  Stats

#pragma region Display_Crafter/RecipeStats

	// Display Crafter/Recipe variables
	SetConsoleTextAttribute(hConsole, 14);
	cout << "--------------------------------------Crafter & Recipe Stats--------------------------------------" << endl << endl;
	cout << "Crafter Level: " << CrafterLevel << "    Craftsmanship: " << Craftsmanship << "      Control: " << Control << "    CP: " << CP << endl;
	cout << "Recipe Level:  " << RecipeLevel << "    Recipe Item Level:" << RecipeItemLevel << "    Progress: " << Recipedifficulty << "   Quality: " << RecipeQualityRequired << "    Durability: " << maxdurability << endl << endl;
	cout << "-----------------------------------End of Crafter & Recipe Stats----------------------------------" << endl << endl;

#pragma endregion

	// Progress

#pragma region Get_ProgressValues

	vector<int> BasicSynthesisValues;
	vector<int> CarefulSynthesisValues;
	vector<int> CarefulSynthesis2Values;
	vector<int> CarefulSynthesis3Values;
	vector<int> FocusedSynthesisValues;

	GetProgressValuesVec(BasicSynthesisValues, BasicSynthesisMultiplier, levelCorrectedProgress, levelCorrectedProgressING1, levelCorrectedProgressING2);
	GetProgressValuesVec(CarefulSynthesisValues, CarefulSynthesis1Multiplier, levelCorrectedProgress, levelCorrectedProgressING1, levelCorrectedProgressING2);
	GetProgressValuesVec(CarefulSynthesis2Values, CarefulSynthesis2Multiplier, levelCorrectedProgress, levelCorrectedProgressING1, levelCorrectedProgressING2);
	GetProgressValuesVec(CarefulSynthesis3Values, CarefulSynthesis3Multiplier, levelCorrectedProgress, levelCorrectedProgressING1, levelCorrectedProgressING2);
	GetProgressValuesVec(FocusedSynthesisValues, FocusedSynthesisMultiplier, levelCorrectedProgress, levelCorrectedProgressING1, levelCorrectedProgressING2);

#pragma endregion

#pragma region Display_ProgressValues
	SetConsoleTextAttribute(hConsole, 13);
	cout << "-------------------------Progress Actions------------------------" << endl << endl;
	printProgressBuffs(levelCorrectedProgress, levelCorrectedProgressING1, levelCorrectedProgressING2);
	cout << "-----------------------------------------------------------------" << endl;
	printProgressValues("Basic Synthesis: ", BasicSynthesisValues, levelCorrectedProgress, levelCorrectedProgressING1, levelCorrectedProgressING2);
	printProgressValues("Careful Synthesis: ", CarefulSynthesisValues, levelCorrectedProgress, levelCorrectedProgressING1, levelCorrectedProgressING2);
	printProgressValues("Careful Synthesis 2: ", CarefulSynthesis2Values, levelCorrectedProgress, levelCorrectedProgressING1, levelCorrectedProgressING2);
	printProgressValues("Careful Synthesis 3: ", CarefulSynthesis3Values, levelCorrectedProgress, levelCorrectedProgressING1, levelCorrectedProgressING2);
	printProgressValues("Focused Synthesis: ", FocusedSynthesisValues, levelCorrectedProgress, levelCorrectedProgressING1, levelCorrectedProgressING2);
	cout << "-----------------------------------------------------------------" << endl;

#pragma region PbP_Progress

	// Calculate & display the value of all piece by piece's till it becomes less reliable than an unbuffed basic synthesis
	int PbPNumber = 0;
	int currentProgressPbP = 0;

	cout << "Piece by Piece Number: \t";
	do
	{
		currentProgressPbP = (currentProgressPbP + ((Recipedifficulty - currentProgressPbP)*PbP));
		PbPNumber++;
		cout << PbPNumber << "\t";
	} while (((Recipedifficulty - currentProgressPbP)*PbP) > BasicSynthesisValues[0]);

	cout << endl;

	int ProgressPbPTotal = 0;
	int ProgressPbP = 0;
	cout << "Piece by Piece: \t";
	do
	{
		ProgressPbP = ((Recipedifficulty - ProgressPbPTotal)*PbP);
		ProgressPbPTotal = (ProgressPbPTotal + ((Recipedifficulty - ProgressPbPTotal)*PbP));
		cout << ProgressPbP << "\t";
	} while (((Recipedifficulty - ProgressPbPTotal)*PbP) > BasicSynthesisValues[0]);

	cout << endl;
	cout << "-----------------------------------------------------------------" << endl;
	cout << endl;

#pragma endregion

	cout << "----------------------End of Progress Actions--------------------" << endl << endl;

#pragma endregion

	// Quality

#pragma region Get_QualityPenalty

	// Used to figure out the Quality penalty tier that will be applies the the base quality
	// Also figures out the level difference between the recipe & penalty tier for a smallier quality penalty

	float QualityLevelCorrector = 0;
	int RecipeLeveltoQualityPenalityDifference = 0;
	float QualityPenaltyTier = 0;

	map<int, float> QualityPenaltyTable;
	map<int, float>::iterator low, prev;

	QualityPenaltyTable[0] = -0.02;
	QualityPenaltyTable[90] = -0.03;
	QualityPenaltyTable[160] = -0.05;
	QualityPenaltyTable[180] = -0.06;
	QualityPenaltyTable[200] = -0.07;
	QualityPenaltyTable[245] = -0.08;
	QualityPenaltyTable[300] = -0.09;
	QualityPenaltyTable[310] = -0.10;
	QualityPenaltyTable[340] = -0.11;

	low = QualityPenaltyTable.lower_bound(RecipeItemLevel);
	prev = std::prev(low);

	// Use last element in the list
	// If recipeItemLevel is higher than the last element
	if (low == QualityPenaltyTable.end())
	{
		RecipeLeveltoQualityPenalityDifference = (RecipeItemLevel - (prev->first));
		QualityPenaltyTier = prev->second;
	}
	// Use first element in the list
	// If recipeItemLevel is lower than the last element
	else if (low == QualityPenaltyTable.begin())
	{
		RecipeLeveltoQualityPenalityDifference = (low->first);
		QualityPenaltyTier = low->second;
	}
	else
	{
		// Set to same element as recipe level
		if ((low->first - RecipeItemLevel) == 0)
		{
			RecipeLeveltoQualityPenalityDifference = (RecipeItemLevel - (low->first));
			QualityPenaltyTier = low->second;
		}
		else
			// Set to previous element 
		{
			RecipeLeveltoQualityPenalityDifference = (RecipeItemLevel - (prev->first));
			QualityPenaltyTier = prev->second;
		}
	}

	QualityLevelCorrector = 0.05 * -RecipeLeveltoQualityPenalityDifference;

	// Set a maximum penalty of 50%
	if (QualityLevelCorrector < 0.5)
	{
		QualityLevelCorrector = -0.5;
	}
	if (RecipeItemLevel <= 290)
	{
		QualityLevelCorrector = 0;
	}
#pragma endregion

#pragma region Get_RecipePenalty
	float Penalty = 0;
	if (RecipeLevel < 50)
	{
		Penalty = (1 + (RecipeLevel*-0.00015 + 0.005));
	}
	else
	{
		Penalty = (1 + (RecipeLeveltoQualityPenalityDifference * -0.0002) + QualityPenaltyTier);
	}
#pragma endregion

#pragma region Get_QualityValues

	//For each quality skill, pass the:
	//- skill multiplier value
	//- a special modifer used to determine the efficency of skills that can vary depending on inner quiet stacks
	//- the level difference between the crafter & recipe for unbuffed
	//- the level difference between the crafter & recipe for ingenuity 1
	//- the level difference between the crafter & recipe for ingenuity 2
	//- the smaller recipe level difference
	//- the quality tier penalty
	//- Recipe level

	// These are passed into a function too calculate all the quality values that particular skill can get under all possible combinations of buffs & for all inner quiet stacks (1-11, 11 being maxed)

	vector<vector<int> > BasicTouchValues;
	vector<vector<int> > StandardTouchValues;
	vector<vector<int> > AdvancedTouchValues;

	vector<vector<int> > BMiracleValues;
	vector<vector<int> > BBrowValues;
	vector<vector<int> > BBlessingValues;

	GetQualityValuesVec(BasicTouchValues, 1, 0, Control, LevelDifference, LevelDifferenceING1, LevelDifferenceING2, Penalty);
	GetQualityValuesVec(StandardTouchValues, 1.25, 0, Control, LevelDifference, LevelDifferenceING1, LevelDifferenceING2, Penalty);
	GetQualityValuesVec(AdvancedTouchValues, 1.5, 0, Control, LevelDifference, LevelDifferenceING1, LevelDifferenceING2, Penalty);
	GetQualityValuesVec(BMiracleValues, 1, 1, Control, LevelDifference, LevelDifferenceING1, LevelDifferenceING2, Penalty);
	GetQualityValuesVec(BBrowValues, 1.5, 2, Control, LevelDifference, LevelDifferenceING1, LevelDifferenceING2, Penalty);
	GetQualityValuesVec(BBlessingValues, 1, 3, Control, LevelDifference, LevelDifferenceING1, LevelDifferenceING2, Penalty);

#pragma endregion

#pragma region Display_QualityValues


	SetConsoleTextAttribute(hConsole, 11);
	cout << "---------------------------------------------------------------------------------------------------Quality Actions-----------------------------------------------------------------------------------------------------" << endl << endl;

	printTableHeader2("Basic Touch: 100% efficiency", "Byregot's Miracle: 100% + 15% efficiency per additional inner quiet stack");
	printVector2(BasicTouchValues, BMiracleValues, LevelDifference, LevelDifferenceING1, LevelDifferenceING2);

	printTableHeader2("Standard Touch: 125% efficiency", "Byregot's Brow: 150% + 10% efficiency per additional inner quiet stack");
	printVector2(StandardTouchValues, BBrowValues, LevelDifference, LevelDifferenceING1, LevelDifferenceING2);

	printTableHeader2("Advanced Touch: 150% efficiency", "Byregot's Blessing: 100% + 20% efficiency per additional inner quiet stack");
	printVector2(AdvancedTouchValues, BBlessingValues, LevelDifference, LevelDifferenceING1, LevelDifferenceING2);

	cout << "-----------------------------------------------------------------------------------------------End of Quality Actions--------------------------------------------------------------------------------------------------" << endl << endl;

#pragma endregion

	// Solver

#pragma region OpenerSolver

	// Initilise  & decide Crafting opener 
	vector<string> Full_Rotation;
	vector<string> IP_Opener;

	// Opener  for level 70
	if (CrafterLevel >= 70)
	{
		IP_Opener = { "Initial Preperations", "Inner Quiet", "Specialty:Reflect", "Steady Hand 2" };
		SHbuff = 5;
		CP -= (50 + 18 + 25);
	}
	// Opener for crafters not  level 70
	else
	{
		IP_Opener = { "Inner Quiet", "Steady Hand 2" };
		SHbuff = 5;
		CP -= (18 + 25);
	}

#pragma endregion

#pragma region ProgressSolver

	int RNGcount = 0;

	//cout << "---- Crafting Sequence Solver (Progress) ----" << endl << endl;

	//cout << "Skill Name" << setw(25) << "Progress" << endl;
	//cout << "-----------------------------------" << endl;

	do
	{

		//#pragma region MuscleMemory
		//
		//		if ((((Recipedifficulty - currentProgress)*PbP) > CarefulSynthesis3Values[2]) && (MuscleMemoryAvailable == true))
		//		{
		//			cout << "Muscle Memory" << endl;
		//			ProgressNames.push_back("Muscle Memory");
		//			currentProgress = (currentProgress + ((Recipedifficulty - currentProgress)*PbP));
		//			MuscleMemoryAvailable = false;
		//			durability -= 10;
		//			CP -= 6;
		//		}
		//
		//#pragma endregion
		//#pragma region PbP
		//		else if ((((Recipedifficulty - currentProgress)*PbP) > CarefulSynthesis3Values[2]) && (PiecebyPieceAvailable == true))
		//		{
		//			cout << "PbP" << endl;
		//			if (SHbuff == 0)
		//			{
		//				ProgressNames.push_back("Steady Hand 2");
		//				cout << ProgressNames[countProgress] << setw(30 - ProgressNames[countProgress].size()) << currentProgress << "/" << Recipedifficulty << endl;
		// 				countProgress++;
		//				SHbuff = 5;
		//				CP -= 25;
		//			}
		//			ProgressNames.push_back("Piece by Piece");
		//			currentProgress = (currentProgress + ((Recipedifficulty - currentProgress)*PbP));
		//			RNGcount++;
		//			SHbuff--;
		//			durability -= 10;
		//			CP -= 15;
		//		}
		//#pragma endregion
#pragma region ING2
		if (Ingenuity2Available == true)
		{
			if (CrafterLevel >= 62)
			{
				///*if (CarefulSynthesis3ProgressING2 > CarefulSynthesis3Progress)*/
				if (CarefulSynthesis3Values[2] > CarefulSynthesis3Values[0])
				{
					if (ING2buff == 0)
					{
						ProgressNames.push_back("Ingenuity 2");
						//cout << ProgressNames[countProgress] << setw(30 - ProgressNames[countProgress].size()) << currentProgress << "/" << Recipedifficulty << endl;
						countProgress++;
						ING2buff = 5;
						CP -= 32;
					}
					ProgressNames.push_back("Careful Synthesis 3");
					currentProgress = (currentProgress + CarefulSynthesis3Values[2]);
					durability -= 10;
					CP -= 7;
				}
				else
				{
					ProgressNames.push_back("Careful Synthesis 3");
					currentProgress = (currentProgress + CarefulSynthesis3Values[0]);
					durability -= 10;
					CP -= 7;
				}
			}
			else if (CrafterLevel >= 50)
			{
				if (CarefulSynthesis2Values[2] > CarefulSynthesis2Values[0])
				{
					if (ING2buff == 0)
					{
						ProgressNames.push_back("Ingenuity 2");
						//cout << ProgressNames[countProgress] << setw(30 - ProgressNames[countProgress].size()) << currentProgress << "/" << Recipedifficulty << endl;
						countProgress++;
						ING2buff = 5;
						CP -= 32;
					}
					ProgressNames.push_back("Careful Synthesis 2");
					currentProgress = (currentProgress + CarefulSynthesis2Values[2]);
					durability -= 10;
				}
				else
				{
					ProgressNames.push_back("Careful Synthesis 2");
					currentProgress = (currentProgress + CarefulSynthesis3Values[0]);
					durability -= 10;
				}
			}
			else if (CrafterLevel >= 15)
			{
				if (CarefulSynthesisValues[2] > CarefulSynthesisValues[0])
				{
					if (ING2buff == 0)
					{
						ProgressNames.push_back("Ingenuity 2");
						//cout << ProgressNames[countProgress] << setw(30 - ProgressNames[countProgress].size()) << currentProgress << "/" << Recipedifficulty << endl;
						countProgress++;
						ING2buff = 5;
						CP -= 32;
					}
					ProgressNames.push_back("Careful Synthesis 1");
					currentProgress = (currentProgress + CarefulSynthesisValues[2]);
					durability -= 10;
				}
				else
				{
					ProgressNames.push_back("Careful Synthesis 1");
					currentProgress = (currentProgress + CarefulSynthesisValues[0]);
					durability -= 10;
				}
			}

		}
#pragma endregion
#pragma region ING1
		else if (Ingenuity1Available == true)
		{
			if (ING1buff == 0)
			{
				ProgressNames.push_back("Ingenuity 1");
				//cout << ProgressNames[countProgress] << setw(30 - ProgressNames[countProgress].size()) << currentProgress << "/" << Recipedifficulty << endl;
				countProgress++;
				ING1buff = 5;
				CP -= 24;
			}
			ProgressNames.push_back("Careful Synthesis 3");
			currentProgress = (currentProgress + CarefulSynthesis3Values[1]);
			durability -= 10;
			CP -= 7;
		}
#pragma endregion
		else
		{
			ProgressNames.push_back("Careful Synthesis 3");
			currentProgress = (currentProgress + CarefulSynthesis3Values[0]);
			durability -= 10;
			CP -= 7;
		}
		//cout << ProgressNames[countProgress] << setw(30 - ProgressNames[countProgress].size()) << currentProgress << "/" << Recipedifficulty << endl;
		countProgress++;
		i++;

#pragma region buffsteps
		if (ING2buff > 0)
		{
			ING2buff--;
		}
		if (ING1buff > 0)
		{
			ING1buff--;
		}
#pragma endregion

	} while ((currentProgress <= Recipedifficulty));

	//cout << endl;

#pragma region Success/NotSuccess
	if (currentProgress >= Recipedifficulty)
	{
		//cout << "Successfully crafted NQ" << endl;
	}
	else
	{
		//cout << "Unable to NQ this recipe" << endl;
	}
#pragma endregion

	//cout << endl;

#pragma endregion

#pragma region QualitySolver

	// 2 vectors to store Quality names & Quality value
	vector<string> vQualityNames;
	vector<int> vQualityValues;


#pragma region Sequence_Touch

	// Determine inner quiet stack
	// At Crafter level 70, initial preperations & Specialty:reflect is used so inner quiet  will start at 4
	if (CrafterLevel == 70)
	{
		i = 3;
	}
	else
	{
		i = 0;
	}

	int BTsum = 0;
	int BBrowFinisher = 0;
	int Total_BT_Brow = 0;

	// Calculate sum of 100% efficency quality skills 
	// Calculate the Finsiher
	// Do till max stacks are reached or the sum of quality skills & finisher if more or equal to the required quality needed
	do
	{
		BTsum += BasicTouchValues[i][0];
		BBrowFinisher = BBrowValues[i + 1][3];
		Total_BT_Brow = (BTsum + BBrowFinisher);
		vQualityValues.push_back(BasicTouchValues[i][0]);


		// Use prudent touch if available
		// Crafter level 66+
		// Reduce CP & durability for the specific skills being used

		if (CrafterLevel >= 66)
		{
			vQualityNames.push_back("Prudent Touch");
			CP -= 21;
			durability -= 5;
		}
		// If not available use Basic touch
		else
		{
			vQualityNames.push_back("Basic Touch");
			CP -= 18;
			durability -= 10;
		}

		// decrement the steady hand 2 buff
		if (SHbuff > 0)
		{
			SHbuff--;
		}

		// Reapply Steady Hand 2 if it has worn off
		if (SHbuff <= 0)
		{
			vQualityNames.push_back("Steady Hand 2");
			vQualityValues.push_back(0);
			SHbuff = 5;
			CP -= 25;
		}

		// Increase inner quiet stack
		i++;

	} while ((Total_BT_Brow < RecipeQualityRequired) && (i < 10));

#pragma endregion


#pragma region Sequence_Finisher
	// Crafting rotation finisher
	// Uses Great strides, Innovation & Byregot's Blessing
	// reduce CP & durability costs for each skill
	if (Total_BT_Brow >= RecipeQualityRequired)
	{
		vQualityNames.push_back("Great Strides");
		vQualityValues.push_back(0);
		CP -= 32;

		vQualityNames.push_back("Innovation");
		vQualityValues.push_back(0);
		CP -= 18;

		vQualityNames.push_back("Byregot's Brow");
		vQualityValues.push_back(BBrowValues[i + 1][3]);
		CP -= 24;

		Total_BT_Brow = Total_BT_Brow - BBrowFinisher + BBlessingValues[i + 1][3];

		durability -= 10;
	}

	//cout << endl;
#pragma endregion

#pragma region Solver_Quality
//	// Display the Crafting sequence ( Not for end user) 
//	
//	cout << "---> Crafting Sequence Solver (Quality) <---" << endl << endl;
//	cout << "Skill Name" << setw(25) << "Quality" << endl;
//	cout << "-----------------------------------" << endl;
//	for (i = 0; i < vQualityNames.size(); i++)
//	{
//		cout << vQualityNames[i] << setw(35 - vQualityNames[i].size()) << vQualityValues[i] << endl;
//
//	}
//	cout << endl;
//
//	cout << "Total Quality: " << Total_BT_Blessing << endl;
//
//	cout << endl;
//	
#pragma endregion

#pragma endregion

#pragma region CompileRotation
	// Using previous vectors populate the final vector to be used to display the crafting  sequence to the user

	//cout << "Opener" << endl << endl;

	for (int i = 0; i < IP_Opener.size(); i++)
	{
		Full_Rotation.push_back(IP_Opener[i]);
		//cout << Full_Rotation[i] << endl;
	}

	//cout << endl << endl << "Progress Skills" << endl << endl;

	for (int i = 0; i < RNGcount; i++)
	{
		Full_Rotation.push_back(ProgressNames[i]);
		//cout << Full_Rotation[i] << endl;
	}

	//cout << endl << endl << "Quality Skills" << endl << endl;

	for (int i = 0; i < vQualityNames.size(); i++)
	{
		Full_Rotation.push_back(vQualityNames[i]);
		//cout << Full_Rotation[i] << endl;
	}

	//cout << endl;

#pragma endregion

	// Tests

#pragma region Test_Durability
	// Test durability. 
	// If durability is more then 0 then it succeeds
	if (durability >= 0)
	{
		//cout << "Succeeded Durability Test" << endl;
		//cout << "Durability left: " << durability << "/" << maxdurability << endl;
	}
	else
	{
		//cout << "Failed durability" << endl;
	}
	cout << endl;

#pragma endregion

#pragma region Test_CP
	// Test CP. 
	// If CP is more then 0 then it succeeds
	if (CP >= 0)
	{
		//cout << "Succeeded CP Test" << endl;
		//cout << "CP left: " << CP << endl;
	}
	else
	{
		//cout << "Failed CP" << endl;
	}
	//cout << endl;
#pragma endregion

	// Show final rotation

#pragma region Complete_Solver

	//Display rotation if successful
	// or
	// Display a message saying "Don't meet the stat requirements to successfully craft this recipe"
	if ((CP >= 0) && (durability >= 0))
	{
		SetConsoleTextAttribute(hConsole, 10);
		cout << "Crafting Sequence Solver" << endl;
		cout << "  (Complete Rotation)   " << endl << endl;
		cout << "Skill Name" << endl; //setw(25) << "Quality" << endl;
		cout << "-------------------" << endl; //----------------" << endl;

		for (int i = RNGcount; i < ProgressNames.size(); i++)
		{
			Full_Rotation.push_back(ProgressNames[i]);
		}

		for (int i = 0; i < Full_Rotation.size(); i++)
		{
			cout << Full_Rotation[i] << endl;
		}
		cout << endl;
		cout << "Successfully crafted HQ" << endl;
	}
	else
	{
		SetConsoleTextAttribute(hConsole, 12);
		cout << "Don't meet the stat requirements to successfully craft this recipe" << endl;
	}

#pragma endregion

	cout << endl << endl;

#pragma region EndAndOutputTime

	SetConsoleTextAttribute(hConsole, 15);
	// Record end time
	auto finishsequential = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finishsequential - startsequential;
	// Output elapsed time
	std::cout << "Total  time:  " << elapsed.count() << " seconds" << std::endl;
#pragma endregion

	system("pause");
}
#pragma endregion

#pragma region Function Definitions

#pragma region UserInput
int userInput_CrafterLevel()
{
	int temp_Crafterlevel;
	do
	{
		cout << "Input your crafter level: ";
		cin >> temp_Crafterlevel;
	} while (temp_Crafterlevel <= 0 || temp_Crafterlevel >= 9999);
	return temp_Crafterlevel;
}
int userInput_Craftsmanship()
{
	int temp_Craftsmanship;
	do
	{
		cout << "Input your craftsmanship: ";
		cin >> temp_Craftsmanship;
	} while (temp_Craftsmanship <= 0 || temp_Craftsmanship >= 9999);
	return temp_Craftsmanship;
}
int userInput_Control()
{
	int temp_Control;
	do
	{
		cout << "Input your control: ";
		cin >> temp_Control;
	} while (temp_Control <= 0 || temp_Control >= 9999);
	return temp_Control;
}
int userInput_CP()
{
	int temp_CP;
	do
	{
		cout << "Input your CP: ";
		cin >> temp_CP;
	} while (temp_CP <= 0 || temp_CP >= 9999);
	return temp_CP;
}
int userInput_RecipeLevel()
{
	int temp_RecipeLevel;
	do
	{
		cout << "Input recipe level: ";
		cin >> temp_RecipeLevel;
	} while (temp_RecipeLevel <= 0 || temp_RecipeLevel >= 9999);
	return temp_RecipeLevel;
}
int userInput_RecipeItemLevel()
{
	int temp_RecipeItemLevel;
	do
	{
		cout << "Input recipe Item level: ";
		cin >> temp_RecipeItemLevel;
	} while (temp_RecipeItemLevel <= 0 || temp_RecipeItemLevel >= 9999);
	return temp_RecipeItemLevel;
}
int userInput_ProgressRequired()
{
	int temp_ProgressRequired;
	do
	{
		cout << "Input recipe progress requirements: ";
		cin >> temp_ProgressRequired;
	} while (temp_ProgressRequired <= 0 || temp_ProgressRequired >= 99999);
	return temp_ProgressRequired;
}
int userInput_QualityRequired()
{
	int temp_QualityRequired;
	do
	{
		cout << "Input recipe quality requirements: ";
		cin >> temp_QualityRequired;
	} while (temp_QualityRequired <= 0 || temp_QualityRequired >= 99999);
	return temp_QualityRequired;
}
int userInput_RecipeDurability()
{
	int temp_RecipeDurability;
	do
	{
		cout << "Input recipe durability: ";
		cin >> temp_RecipeDurability;
	} while (temp_RecipeDurability <= 0 || temp_RecipeDurability >= 100);
	return temp_RecipeDurability;
}
#pragma endregion

// Lookup the Crafter Level Recipe item level equivilent
int GetCrafterLevelToItemLevelequivalent(int crafterLevel)
{
#pragma region CrafterLevelToItemLevelequivalent

	int CrafterLevelToItemLevelequivalent = 0;
	map<int, int> CrafterLevelTable;

	CrafterLevelTable[51] = 120;
	CrafterLevelTable[52] = 125;
	CrafterLevelTable[53] = 130;
	CrafterLevelTable[54] = 133;
	CrafterLevelTable[55] = 136;
	CrafterLevelTable[56] = 139;
	CrafterLevelTable[57] = 142;
	CrafterLevelTable[58] = 145;
	CrafterLevelTable[59] = 148;
	CrafterLevelTable[60] = 250;
	CrafterLevelTable[61] = 260;
	CrafterLevelTable[62] = 265;
	CrafterLevelTable[63] = 270;
	CrafterLevelTable[64] = 273;
	CrafterLevelTable[65] = 276;
	CrafterLevelTable[66] = 279;
	CrafterLevelTable[67] = 282;
	CrafterLevelTable[68] = 285;
	CrafterLevelTable[69] = 288;
	CrafterLevelTable[70] = 290;

	if (crafterLevel < 50)
	{
		CrafterLevelToItemLevelequivalent = crafterLevel;
	}
	else
	{
		CrafterLevelToItemLevelequivalent = CrafterLevelTable[crafterLevel];
	}

	return CrafterLevelToItemLevelequivalent;

	//	cout << "CrafterLevel to ItemLevel equivalent: " << CrafterLevelToItemLevelequivalent << endl;

#pragma endregion
}

// Lookup the Crafter Level Recipe item level equivilent (While under the Ingenuity 1 buff)
int GetCrafterLevelToItemLevelequivalentING1(int RecipeItemLevel)
{
#pragma region Ingenuity1Table

	int recipeItemLevel = RecipeItemLevel;
	map<int, int> Ing1RecipeLevelTable;

	Ing1RecipeLevelTable[40] = 36;
	Ing1RecipeLevelTable[41] = 36;
	Ing1RecipeLevelTable[42] = 37;
	Ing1RecipeLevelTable[43] = 38;
	Ing1RecipeLevelTable[44] = 39;
	Ing1RecipeLevelTable[45] = 40;
	Ing1RecipeLevelTable[46] = 41;
	Ing1RecipeLevelTable[47] = 42;
	Ing1RecipeLevelTable[48] = 43;
	Ing1RecipeLevelTable[49] = 44;
	Ing1RecipeLevelTable[50] = 45;
	Ing1RecipeLevelTable[55] = 50;		// 50_1star     *** unverified
	Ing1RecipeLevelTable[70] = 51;		// 50_2star     *** unverified
	Ing1RecipeLevelTable[90] = 58;		// 50_3star     *** unverified
	Ing1RecipeLevelTable[110] = 59;		// 50_4star     *** unverified
	Ing1RecipeLevelTable[115] = 100;	// 51 @ 169/339 difficulty
	Ing1RecipeLevelTable[120] = 101;	// 51 @ 210/410 difficulty
	Ing1RecipeLevelTable[125] = 102;	// 52
	Ing1RecipeLevelTable[130] = 110;	// 53
	Ing1RecipeLevelTable[133] = 111;	// 54
	Ing1RecipeLevelTable[136] = 112;	// 55
	Ing1RecipeLevelTable[139] = 126;	// 56
	Ing1RecipeLevelTable[142] = 131;	// 57
	Ing1RecipeLevelTable[145] = 134;	// 58
	Ing1RecipeLevelTable[148] = 137;	// 59
	Ing1RecipeLevelTable[150] = 140;	// 60
	Ing1RecipeLevelTable[160] = 151;	// 60_1star
	Ing1RecipeLevelTable[180] = 152;	// 60_2star
	Ing1RecipeLevelTable[210] = 153;	// 60_3star
	Ing1RecipeLevelTable[220] = 153;	// 60_3star
	Ing1RecipeLevelTable[250] = 154;	// 60_4star
	Ing1RecipeLevelTable[255] = 238;	// 61 @ 558/1116 difficulty
	Ing1RecipeLevelTable[260] = 240;	// 61 @ 700/1400 difficulty
	Ing1RecipeLevelTable[265] = 242;	// 62
	Ing1RecipeLevelTable[270] = 250;	// 63
	Ing1RecipeLevelTable[273] = 251;	// 64
	Ing1RecipeLevelTable[276] = 252;	// 65
	Ing1RecipeLevelTable[279] = 266;	// 66
	Ing1RecipeLevelTable[282] = 271;	// 67
	Ing1RecipeLevelTable[285] = 274;	// 68
	Ing1RecipeLevelTable[288] = 277;	// 69
	Ing1RecipeLevelTable[290] = 280;	// 70
	Ing1RecipeLevelTable[300] = 291;    // 70_1star
	Ing1RecipeLevelTable[320] = 292;    // 70_1star
	Ing1RecipeLevelTable[350] = 293;    // 70_1star

	if (recipeItemLevel < 40)
	{
		recipeItemLevel = recipeItemLevel - 5;
	}
	else
	{
		recipeItemLevel = Ing1RecipeLevelTable[recipeItemLevel];
	}
	////cout << "CrafterLevel to ItemLevel equivalent (ING1): " << CrafterLevelToItemLevelequivalentING1 << endl;

	return recipeItemLevel;


#pragma endregion
}

// Lookup the Crafter Level Recipe item level equivilent (While under the Ingenuity 2 buff)
int GetCrafterLevelToItemLevelequivalentING2(int RecipeItemLevel)
{
#pragma region Ingenuity2Table

	int recipeItemLevel = RecipeItemLevel;
	map<int, int> Ing2RecipeLevelTable;

	Ing2RecipeLevelTable[40] = 33;
	Ing2RecipeLevelTable[41] = 34;
	Ing2RecipeLevelTable[42] = 35;
	Ing2RecipeLevelTable[43] = 36;
	Ing2RecipeLevelTable[44] = 37;
	Ing2RecipeLevelTable[45] = 38;
	Ing2RecipeLevelTable[46] = 39;
	Ing2RecipeLevelTable[47] = 40;
	Ing2RecipeLevelTable[48] = 40;
	Ing2RecipeLevelTable[49] = 41;
	Ing2RecipeLevelTable[50] = 42;
	Ing2RecipeLevelTable[55] = 47;		// 50_1star     *** unverified
	Ing2RecipeLevelTable[70] = 48;		// 50_2star     *** unverified
	Ing2RecipeLevelTable[90] = 56;		// 50_3star     *** unverified
	Ing2RecipeLevelTable[110] = 57;		// 50_4star     *** unverified
	Ing2RecipeLevelTable[115] = 97;		// 51 @ 169/339 difficulty
	Ing2RecipeLevelTable[120] = 99;		// 51 @ 210/410 difficulty
	Ing2RecipeLevelTable[125] = 101;	// 52
	Ing2RecipeLevelTable[130] = 109;	// 53
	Ing2RecipeLevelTable[133] = 110;	// 54
	Ing2RecipeLevelTable[136] = 111;	// 55
	Ing2RecipeLevelTable[139] = 125;	// 56
	Ing2RecipeLevelTable[142] = 130;	// 57
	Ing2RecipeLevelTable[145] = 133;	// 58
	Ing2RecipeLevelTable[148] = 136;	// 59
	Ing2RecipeLevelTable[150] = 139;	// 60
	Ing2RecipeLevelTable[160] = 150;	// 60_1star
	Ing2RecipeLevelTable[180] = 151;	// 60_2star
	Ing2RecipeLevelTable[210] = 152;	// 60_3star
	Ing2RecipeLevelTable[220] = 152;	// 60_3star
	Ing2RecipeLevelTable[250] = 153;	// 60_4star
	Ing2RecipeLevelTable[255] = 237;	// 61 @ 558/1116 difficulty
	Ing2RecipeLevelTable[260] = 239;	// 61 @ 700/1400 difficulty
	Ing2RecipeLevelTable[265] = 241;	// 62
	Ing2RecipeLevelTable[270] = 249;	// 63
	Ing2RecipeLevelTable[273] = 250;	// 64
	Ing2RecipeLevelTable[276] = 251;	// 65
	Ing2RecipeLevelTable[279] = 265;	// 66
	Ing2RecipeLevelTable[282] = 270;	// 67
	Ing2RecipeLevelTable[285] = 273;	// 68
	Ing2RecipeLevelTable[288] = 276;	// 69
	Ing2RecipeLevelTable[290] = 279;	// 70
	Ing2RecipeLevelTable[300] = 290;    // 70_1star
	Ing2RecipeLevelTable[320] = 291;    // 70_1star
	Ing2RecipeLevelTable[350] = 292;    // 70_1star

	if (recipeItemLevel < 40)
	{
		recipeItemLevel = recipeItemLevel - 7;
	}
	else
	{
		recipeItemLevel = Ing2RecipeLevelTable[recipeItemLevel];
	}
	//	cout << "CrafterLevel to ItemLevel equivalent (ING2): " << CrafterLevelToItemLevelequivalentING2 << endl;

	return recipeItemLevel;

#pragma endregion
}

#pragma region GetProgressValuesVec
// Calculate base progress for unbuffed, ingenuity 1 & ingenuity 2
float calculateBaseProgressIncrease(int levelDifference, int craftsmanship, int CrafterLevelToItemLevelequivalent, int recipeItemLevel)
{
#pragma region Variables
	float baseProgress = 0;
	float recipeLevelPenalty = 0;
	float levelCorrectionFactor = 0;
	float levelCorrectedProgress = 0;
#pragma endregion

#pragma region ProgressPenaltyTier
	map<int, float> ProgressPenaltyTable;
	ProgressPenaltyTable[180] = -0.02;
	ProgressPenaltyTable[210] = -0.035;
	ProgressPenaltyTable[220] = -0.035;
	ProgressPenaltyTable[250] = -0.04;
	ProgressPenaltyTable[320] = -0.02;
	ProgressPenaltyTable[350] = -0.035;
#pragma endregion

#pragma region baseProgress
	if (CrafterLevelToItemLevelequivalent > 250) {
		baseProgress = 1.834712812e-5 * craftsmanship * craftsmanship + 1.904074773e-1 * craftsmanship + 1.544103837;
	}
	else if (CrafterLevelToItemLevelequivalent > 110) {
		baseProgress = 2.09860e-5 * craftsmanship * craftsmanship + 0.196184 * craftsmanship + 2.68452;
	}
	else {
		baseProgress = 0.214959 * craftsmanship + 1.6;
	}
#pragma endregion

#pragma region LevelBoostForRecipesBelowCrafterLevel
	// Level boost for recipes below crafter level
	if (levelDifference > 0) {
		levelCorrectionFactor += (0.25 / 5) * min(levelDifference, 5);
	}
	if (levelDifference > 5) {
		levelCorrectionFactor += (0.10 / 5) * min(levelDifference - 5, 10);
	}
	if (levelDifference > 15) {
		levelCorrectionFactor += (0.05 / 5) * min(levelDifference - 15, 5);
	}
	if (levelDifference > 20) {
		levelCorrectionFactor += 0.0006 * (levelDifference - 20);
	}
#pragma endregion

#pragma region LevelPenaltyForRecipesAboveCrafterLevel
	// Level penalty for recipes above crafter level
	if (levelDifference < 0) {
		levelCorrectionFactor += 0.025 * max(levelDifference, -10);

		if (ProgressPenaltyTable[recipeItemLevel])
		{
			recipeLevelPenalty += ProgressPenaltyTable[recipeItemLevel];
		}
	}

#pragma endregion

	// Level factor is rounded to nearest percent
	levelCorrectionFactor = (floor(levelCorrectionFactor * 100) / 100);
	levelCorrectedProgress = baseProgress * (1 + levelCorrectionFactor) * (1 + recipeLevelPenalty);
	return levelCorrectedProgress;
};
#pragma endregion

#pragma region GetQualityValuesVec
//For each quality skill, pass the:
//- skill multiplier value
//- a special modifer used to determine the efficency of skills that can vary depending on inner quiet stacks
//- the level difference between the crafter & recipe for unbuffed
//- the level difference between the crafter & recipe for ingenuity 1
//- the level difference between the crafter & recipe for ingenuity 2
//- the penalty
// These are passed into a function too calculate all the quality values that particular skill can get under all possible combinations of buffs & for all inner quiet stacks (1-11, 11 being maxed)

void GetQualityValuesVec(vector<vector<int> >&vec, float SkillMultiplier, float FinisherISMultiplier, int Control, float LevelDifference, float LevelDifferenceING1, float LevelDifferenceING2, float Penalty)
{
	int control = Control;
	float penalty = Penalty;

#pragma region LevelDifferences
	float levelDifference = LevelDifference;
	float levelDifferenceING1 = LevelDifferenceING1;
	float levelDifferenceING2 = LevelDifferenceING2;

#pragma endregion

	float skillMultiplier = SkillMultiplier;
	float innovation = 0.5;
	int greatStrides = 2;

	float j = 1;

#pragma region FinisherMultipliers

	for (int i = 0; i < 12; ++i)
	{
		float random = FinisherISMultiplier;
		if (random == 1)
		{
			random = (0.15*i);
		}
		else if (random == 2)
		{
			random = (0.1*i);
		}
		else if (random == 3)
		{
			random = (0.2*i);
		}

#pragma endregion

		vector<int> temp;

		// Basic Touch
		/* No buffs */		temp.push_back(floor((skillMultiplier + random)					*((3.46e-5 * (j					* control) * (j					* control) + 0.3514 * (j				* control) + 34.66)	* levelDifference		* penalty)));
		/* IN       */		temp.push_back(floor((skillMultiplier + random)					*((3.46e-5 * ((j + innovation)	* control) * ((j + innovation)  * control) + 0.3514 * ((j + innovation) * control) + 34.66)	* levelDifference		* penalty)));
		/* GS       */		temp.push_back(floor((skillMultiplier + random)	*(greatStrides	*((3.46e-5 * (j					* control) * (j					* control) + 0.3514 * (j				* control) + 34.66)	* levelDifference		* penalty))));
		/* GS + IN     */	temp.push_back(floor((skillMultiplier + random)	*(greatStrides	*((3.46e-5 * ((j + innovation)	* control) * ((j + innovation)  * control) + 0.3514 * ((j + innovation) * control) + 34.66)	* levelDifference		* penalty))));

		if (levelDifferenceING1 > levelDifference)
		{
			/* ING1     */		temp.push_back(floor((skillMultiplier + random)					*((3.46e-5 * (j					* control) * (j					* control) + 0.3514 * (j				* control) + 34.66)	* levelDifferenceING1	* penalty)));
			/* ING1 + IN   */	temp.push_back(floor((skillMultiplier + random)					*((3.46e-5 * ((j + innovation)	* control) * ((j + innovation)  * control) + 0.3514 * ((j + innovation) * control) + 34.66)	* levelDifferenceING1	* penalty)));
			/* ING1 + GS   */	temp.push_back(floor((skillMultiplier + random)	*(greatStrides	*((3.46e-5 * (j					* control) * (j					* control) + 0.3514 * (j				* control) + 34.66)	* levelDifferenceING1	* penalty))));
			/* ING1 + IN + GS*/	temp.push_back(floor((skillMultiplier + random)	*(greatStrides * ((3.46e-5 * ((j + innovation)	* control) * ((j + innovation)  * control) + 0.3514 * ((j + innovation) * control) + 34.66)	* levelDifferenceING1	* penalty))));
		}

		if (levelDifferenceING2 > levelDifferenceING1)
		{
			/* ING2     */		temp.push_back(floor((skillMultiplier + random)					*((3.46e-5 * (j					* control) * (j					* control) + 0.3514 * (j				* control) + 34.66)	* levelDifferenceING2	* penalty)));
			/* ING2 + IN   */	temp.push_back(floor((skillMultiplier + random)					*((3.46e-5 * ((j + innovation)	* control) * ((j + innovation)  * control) + 0.3514 * ((j + innovation) * control) + 34.66)	* levelDifferenceING2	* penalty)));
			/* ING2 + GS   */	temp.push_back(floor((skillMultiplier + random)	*(greatStrides * ((3.46e-5 * (j					* control) * (j					* control) + 0.3514 * (j				* control) + 34.66)	* levelDifferenceING2	* penalty))));
			/* ING2 + IN + GS*/	temp.push_back(floor((skillMultiplier + random)	*(greatStrides * ((3.46e-5 * ((j + innovation)	* control) * ((j + innovation)  * control) + 0.3514 * ((j + innovation) * control) + 34.66)	* levelDifferenceING2	* penalty))));
		}

		vec.push_back(temp);

		j += 0.2;
	}
}
#pragma endregion 

#pragma region PrintQualityStackHeader
// print skill formatting
void printTableHeader(string TouchType)
{
	cout << TouchType << "\t\t\t\t\t\t\t\t\t\t\t\t\t" << endl;
	cout << "Stack           " << "1-2\t" << "2-3\t" << "3-4\t" << "4-5\t" << "5-6\t" << "6-7\t" << "7-8\t" << "8-9\t" << "9-10\t" << "10-11\t" << "MAX" << endl;
	cout << "-------------------------------------------------------------------------------------------------------" << endl;
}
#pragma endregion 

#pragma region PrintQualityStackHeader2
// print skill formatting
void printTableHeader2(string TouchType, string TouchType2)
{
	cout << TouchType << "\t\t\t\t\t\t\t\t\t" << "\t\t" << TouchType2 << "\t\t\t\t\t\t\t\t\t" << endl;
	cout << "Stack           " << "1-2\t" << "2-3\t" << "3-4\t" << "4-5\t" << "5-6\t" << "6-7\t" << "7-8\t" << "8-9\t" << "9-10\t" << "10-11\t" << "MAX" << "\t\t" << "Stack           " << "1-2\t" << "2-3\t" << "3-4\t" << "4-5\t" << "5-6\t" << "6-7\t" << "7-8\t" << "8-9\t" << "9-10\t" << "10-11\t" << "MAX" << endl;
	cout << "-------------------------------------------------------------------------------------------------------" << "\t\t" << "-------------------------------------------------------------------------------------------------------" << endl;
}
#pragma endregion 

#pragma region PrintVector2
// print skill values
void printVector2(vector<vector<int> > &vec, vector<vector<int> > &vec2, int levelDifference, int levelDifferenceING1, int levelDifferenceING2)
{

#pragma region SkillNames


	vector<string> SkillNames;
	int N = vec.at(0).size();

	//cout << "Vector Size: " << N << endl;

	SkillNames.push_back("No buffs:       ");
	SkillNames.push_back("Innovation:     ");
	SkillNames.push_back("Great Strides:  ");
	SkillNames.push_back("GS + IN:        ");

	if (N > 4)
	{
		SkillNames.push_back("ING1:           ");
		SkillNames.push_back("ING1 + IN:      ");
		SkillNames.push_back("ING1 + GS:      ");
		SkillNames.push_back("ING1 + IN + GS: ");

	}

	if (N > 8)
	{
		SkillNames.push_back("ING2:           ");
		SkillNames.push_back("ING2 + In:      ");
		SkillNames.push_back("ING2 + GS:      ");
		SkillNames.push_back("ING2 + IN + GS: ");
	}

	for (int i = 0; i < N; i++)
	{
		cout << SkillNames[i];

		for (int j = 0; j < 11; j++)
		{
			cout << vec[j][i] << "\t";
		}

		cout << "\t";

		cout << SkillNames[i];

		for (int k = 0; k < 11; k++)
		{
			cout << vec2[k][i] << "\t";
		}

		cout << endl;
	}

	cout << endl;

#pragma endregion

}
#pragma endregion 

void GetProgressValuesVec(vector<int> &vec, float SkillMultiplier, float LevelCorrectedProgress, float LevelCorrectedProgressING1, float LevelCorrectedProgressING2)
{
	float levelCorrectedProgress = LevelCorrectedProgress;
	float levelCorrectedProgressING1 = LevelCorrectedProgressING1;
	float levelCorrectedProgressING2 = LevelCorrectedProgressING2;
	float skillMultiplier = SkillMultiplier;

	/* Basic Synthesis  */ vec.push_back(levelCorrectedProgress*skillMultiplier);

	if (levelCorrectedProgressING1 > levelCorrectedProgress)
	{
		/* Basic Synthesis ING1  */	   vec.push_back(levelCorrectedProgressING1*skillMultiplier);
	}

	if (levelCorrectedProgressING2 > levelCorrectedProgressING1)
	{
		/* Basic Synthesis ING2  */	vec.push_back(levelCorrectedProgressING2*skillMultiplier);
	}

}

#pragma region PrintVector
// print skill values
void printVector(vector<vector<int> > &vec, int levelDifference, int levelDifferenceING1, int levelDifferenceING2)
{

#pragma region SkillNames


	vector<string> SkillNames;
	int N = vec.at(0).size();

	//cout << "Vector Size: " << N << endl;

	SkillNames.push_back("No buffs:       ");
	SkillNames.push_back("Innovation:     ");
	SkillNames.push_back("Great Strides:  ");
	SkillNames.push_back("GS + IN:        ");

	if (N > 4)
	{
		SkillNames.push_back("ING1:           ");
		SkillNames.push_back("ING1 + IN:      ");
		SkillNames.push_back("ING1 + GS:      ");
		SkillNames.push_back("ING1 + IN + GS: ");

	}

	if (N > 8)
	{
		SkillNames.push_back("ING2:           ");
		SkillNames.push_back("ING2 + In:      ");
		SkillNames.push_back("ING2 + GS:      ");
		SkillNames.push_back("ING2 + IN + GS: ");
	}

	for (int i = 0; i < N; i++)
	{
		cout << SkillNames[i];

		for (int j = 0; j < 11; j++)
		{
			cout << vec[j][i] << "\t";
		}
		cout << endl;
	}

	cout << endl;

#pragma endregion





}
#pragma endregion 

void printProgressBuffs(float LevelCorrectedProgress, float LevelCorrectedProgressING1, float LevelCorrectedProgressING2)
{

	float levelCorrectedProgress = LevelCorrectedProgress;
	float levelCorrectedProgressING1 = LevelCorrectedProgressING1;
	float levelCorrectedProgressING2 = LevelCorrectedProgressING2;

	// Display the values of all Buff headers
	vector<string> ProgressBuffs;

	cout << "Progress Table: \t";
	ProgressBuffs.push_back("No Buffs");

	if (levelCorrectedProgressING1 > levelCorrectedProgress)
	{
		ProgressBuffs.push_back("Ingenuity 1");
	}

	if (levelCorrectedProgressING2 > levelCorrectedProgressING1)
	{
		ProgressBuffs.push_back("Ingenuity 2");
	}

	for (int i = 0; i < ProgressBuffs.size(); i++)
	{
		cout << ProgressBuffs[i] << "\t";
	}

	cout << endl;
}

void printProgressValues(string SkillName, vector<int> &vec, float LevelCorrectedProgress, float LevelCorrectedProgressING1, float LevelCorrectedProgressING2)
{
	string skillName = SkillName;
	float levelCorrectedProgress = LevelCorrectedProgress;
	float levelCorrectedProgressING1 = LevelCorrectedProgressING1;
	float levelCorrectedProgressING2 = LevelCorrectedProgressING2;

	cout << skillName << "\t";

	for (int i = 0; i < vec.size(); i++)
	{
		cout << vec[i] << "\t\t";
	}

	cout << endl;
}


#pragma endregion 
