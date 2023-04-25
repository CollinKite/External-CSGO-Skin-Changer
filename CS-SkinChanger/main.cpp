#include "memory.h"
#include <thread>
#include <iostream>
#include <array>

using namespace std;

//junk code
int garb1(int x)
{
	int y = x;
	y = 10 - 10;
	x = y - 293;
	return x;
}

// This method randomly generates a float value between 0 and 1
float randomFloat() {
	return (float)rand() / RAND_MAX;
}

// This method converts a string to an integer by adding up the ASCII values of its characters
int stringToInt(std::string str) {
	int result = 0;
	for (int i = 0; i < str.length(); i++) {
		result += (int)str[i];
	}
	return result;
}

// This method creates a new matrix by adding up the elements of two matrices
int** addMatrices(int** matrix1, int** matrix2, int rows, int cols) {
	int** result = new int* [rows];
	for (int i = 0; i < rows; i++) {
		result[i] = new int[cols];
		for (int j = 0; j < cols; j++) {
			result[i][j] = matrix1[i][j] + matrix2[i][j];
		}
	}
	return result;
}

// This method recursively computes the factorial of a given integer
int factorial(int n) {
	if (n <= 1) {
		return 1;
	}
	return n * factorial(n - 1);
}

namespace offset
{
	//client
	constexpr ::std::ptrdiff_t dwLocalPlayer = 0xDEA964;
	constexpr ::std::ptrdiff_t dwEntityList = 0x4DFFFB4;
	constexpr ::std::ptrdiff_t dwClientState = 0x59F19C;

	//player
	constexpr ::std::ptrdiff_t m_hMyWeapons = 0x2E08;

	//base attributes
	constexpr ::std::ptrdiff_t m_flFallbackWear = 0x31E0;
	constexpr ::std::ptrdiff_t m_nFallbackPaintKit = 0x31D8;
	constexpr ::std::ptrdiff_t m_nFallbackSeed = 0x31DC;
	constexpr ::std::ptrdiff_t m_nFallbackStatTrak = 0x31E4;
	constexpr ::std::ptrdiff_t m_iItemDefinitionIndex = 0x2FBA;
	constexpr ::std::ptrdiff_t m_iItemIDHigh = 0x2FD0;
	constexpr ::std::ptrdiff_t m_iEntityQuality = 0x2FBC;
	constexpr ::std::ptrdiff_t m_iAccountID = 0x2FD8;
	constexpr ::std::ptrdiff_t m_OriginalOwnerXuidLow = 0x31D0;
}

constexpr const int GetWeaponPaint(const short& itemdefinition)
{
	//item id's - https://pastebin.com/raw/3zNVRK4W
	//skin id's - https://totalcsgo.com/skin-ids
	switch (itemdefinition)
	{
		//Rifles
		case 7: return 433; //AK-47 - Neon Rider
		case 9: return 344; // AWP - Dragonlore
		//Pistols
		case 4: return 437; //Glock - Twilight Galaxy	
		case 61:return 504; //USP-S - Kill Confirmed
		default:return 0;
	}

}

int main()
{
		cout << "Press enter to start" << endl;
		cin.get();
		const auto memory = Memory("csgo.exe");
		const auto client = memory.GetModuleAddress("client.dll");

		// Junk Code Call
		float f = randomFloat();

		const auto engine = memory.GetModuleAddress("engine.dll");

		// Junk Code Call
		std::string str = "hello world";
		int i = stringToInt(str);

		if (!client || !engine)
		{
			cout << "Failed to get module address" << endl;
			return 0;
		}

		// Junk Code Call
		int rows = 2;
		int cols = 2;
		int** matrix1 = new int* [rows];
		int** matrix2 = new int* [rows];
		for (int i = 0; i < rows; i++) {
			matrix1[i] = new int[cols];
			matrix2[i] = new int[cols];
			for (int j = 0; j < cols; j++) {
				matrix1[i][j] = i + j;
				matrix2[i][j] = i * j;
			}
		}
		int** result = addMatrices(matrix1, matrix2, rows, cols);
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				std::cout << result[i][j] << " ";
			}
			std::cout << std::endl;
		}

		//print in hex format
		cout << "Client base: 0x" << hex << client << endl;
		cout << "Engine base: 0x" << hex << engine << endl;
		while (true)
		{
			//sleep 2 seconds
			//this_thread::sleep_for(chrono::milliseconds(2));

			//get local player
			const auto& localPlayer = memory.Read<uintptr_t>(client + offset::dwLocalPlayer);
			garb1(5);

			//Read weapon array
			const auto weapons = memory.Read<array<unsigned long, 8>>(localPlayer + offset::m_hMyWeapons);

			bool update = false;
			garb1(5222);

			for (const auto& handle : weapons)
			{
				//get weapon
				const auto& weapon = memory.Read<uintptr_t>((client + offset::dwEntityList + (handle & 0xFFF) * 0x10) - 0x10);

				//check if weapon is valid
				if (!weapon)
					continue;

				//check if we want to apply a skin to the current weapon based off of switch statement
				if (const auto paint = GetWeaponPaint(memory.Read<short>(weapon + offset::m_iItemDefinitionIndex)))
				{
					//compare current weapon
					bool check = memory.Read<int32_t>(weapon + offset::m_nFallbackPaintKit) != paint;
					if (check)
					{
						update = true;
					}

					//set weapon to fallback value (-1)
					memory.Write<int32_t>(weapon + offset::m_iItemIDHigh, -1);

					//set weapon paint kit
					memory.Write<int32_t>(weapon + offset::m_nFallbackPaintKit, paint);

					//set wear (factory new)
					memory.Write<float>(weapon + offset::m_flFallbackWear, 0.1f);

					//add stattrak
					memory.Write<int32_t>(weapon + offset::m_nFallbackStatTrak, 6969);

					//fix stattrack error by adding account id
					memory.Write<int32_t>(weapon + offset::m_iAccountID, memory.Read<int32_t>(weapon + offset::m_OriginalOwnerXuidLow));
				}
			}
			if (update)
			{
				//Force update setting delta tick to -1
				memory.Write<int32_t>(memory.Read<uintptr_t>(engine + offset::dwClientState) + 0x174, -1);
				update = false;
			}


		}
	return 0;
}