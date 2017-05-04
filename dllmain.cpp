#include <cmath>
#include <cstdint>
#include <Windows.h>

class CVector
{
public:
	float x, y, z;
};

class CShinyTexts
{
public:
	CVector vec1;
	CVector vec2;
	CVector vec3;
	CVector vec4;
	float unk[8];
	float distance;
	uint8_t type;
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

static uint32_t jumpAddress = 0x00464BD9;
static uint32_t shinyTextsJumpEnd = 0x00465BB8;
static auto LightForPeds = (uint8_t(__cdecl *)(void))0x00465BE0;
static auto ml = (CVector *(__cdecl *)(CVector *, void *, const CVector &))0x004DFF20;
static uint32_t TheCamera = 0x007E4688;
static CShinyTexts *aShinyTexts = reinterpret_cast<CShinyTexts *>(0x0094A218);
static uint32_t &nTimeInMilliseconds = *reinterpret_cast<uint32_t *>(0x00974B2C);
static uint32_t &NumShinyTexts = *reinterpret_cast<uint32_t *>(0x00978618);

void RegisterOne(CVector vec1, CVector vec2, CVector vec3, CVector vec4, float f0, float f1, float f2, float f3, float f4, float f5, float f6, float f7, uint8_t type, uint8_t r, uint8_t g, uint8_t b, float maxDistance)
{
	aShinyTexts[NumShinyTexts].distance = sqrt(
		pow((vec1.x - *reinterpret_cast<float *>(TheCamera + 0x30)), 2) +
		pow((vec1.y - *reinterpret_cast<float *>(TheCamera + 0x34)), 2) +
		pow((vec1.z - *reinterpret_cast<float *>(TheCamera + 0x38)), 2));
	if (aShinyTexts[NumShinyTexts].distance <= maxDistance) {
		aShinyTexts[NumShinyTexts].vec1 = vec1;
		aShinyTexts[NumShinyTexts].vec2 = vec2;
		aShinyTexts[NumShinyTexts].vec3 = vec3;
		aShinyTexts[NumShinyTexts].vec4 = vec4;
		aShinyTexts[NumShinyTexts].unk[0] = f0;
		aShinyTexts[NumShinyTexts].unk[1] = f1;
		aShinyTexts[NumShinyTexts].unk[2] = f2;
		aShinyTexts[NumShinyTexts].unk[3] = f3;
		aShinyTexts[NumShinyTexts].unk[4] = f4;
		aShinyTexts[NumShinyTexts].unk[5] = f5;
		aShinyTexts[NumShinyTexts].unk[6] = f6;
		aShinyTexts[NumShinyTexts].unk[7] = f7;
		aShinyTexts[NumShinyTexts].type = type;
		aShinyTexts[NumShinyTexts].r = r;
		aShinyTexts[NumShinyTexts].g = g;
		aShinyTexts[NumShinyTexts].b = b;
		if (aShinyTexts[NumShinyTexts].distance > maxDistance / 2.0f) {
			float fade = 2.0f - 2.0f * aShinyTexts[NumShinyTexts].distance / maxDistance;
			aShinyTexts[NumShinyTexts].r = static_cast<uint8_t>(static_cast<float>(r) * fade);
			aShinyTexts[NumShinyTexts].g = static_cast<uint8_t>(static_cast<float>(g) * fade);
			aShinyTexts[NumShinyTexts].b = static_cast<uint8_t>(static_cast<float>(b) * fade);
		}
		NumShinyTexts++;
	}
}

void RegisterOneCall(void *mat)
{
	CVector vec1 = {}, vec2 = {}, vec3 = {}, vec4 = {}, in1, in2, in3, in4;
	if (LightForPeds() == 2) {
		// don't walk
		in1 = { 2.7, 0.706, -0.127 };
		in2 = { 2.7, 1.256, -0.127 };
		in3 = { 2.7, 0.706, -0.40166664 };
		in4 = { 2.7, 1.256, -0.40166664 };
		ml(&vec1, mat, in1);
		ml(&vec2, mat, in2);
		ml(&vec3, mat, in3);
		ml(&vec4, mat, in4);
		RegisterOne(vec1, vec2, vec3, vec4, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1, 255, 0, 0, 60.0);
	} else if (LightForPeds() == 0 || nTimeInMilliseconds & 0x100) {
		// walk
		in1 = { 2.7, 0.706, -0.40166664 };
		in2 = { 2.7, 1.256, -0.40166664 };
		in3 = { 2.7, 0.706, -0.539 };
		in4 = { 2.7, 1.256, -0.539 };
		ml(&vec1, mat, in1);
		ml(&vec2, mat, in2);
		ml(&vec3, mat, in3);
		ml(&vec4, mat, in4);
		RegisterOne(vec1, vec2, vec3, vec4, 1.0, 0.5, 0.0, 0.5, 1.0, 1.0, 0.0, 1.0, 1, 255, 255, 255, 60.0);
	}
}

void __declspec(naked) RegisterOneCallProxy()
{
	__asm
	{
		lea edi, [edi+4]
		push edi
		call RegisterOneCall
		add esp, 4
		jmp shinyTextsJumpEnd
	}
}

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH) {
		if (*reinterpret_cast<uint32_t *>(jumpAddress) == 0x000FDAE9) {
			DWORD flOldProtect;
			VirtualProtect(reinterpret_cast<void *>(jumpAddress + 1), 4, PAGE_EXECUTE_READWRITE, &flOldProtect);
			*reinterpret_cast<uint32_t *>(jumpAddress + 1) = reinterpret_cast<uint32_t>(&RegisterOneCallProxy) - jumpAddress - 5;
			VirtualProtect(reinterpret_cast<void *>(jumpAddress + 1), 4, flOldProtect, &flOldProtect);
		}
	}
	return TRUE;
}
