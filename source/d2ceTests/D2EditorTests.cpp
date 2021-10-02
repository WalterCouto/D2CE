#include "pch.h"
#include "CppUnitTest.h"
#include <algorithm>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace D2EditorTests
{
    static std::string& ConvertNewLines(std::string& data)
    {
        auto pos = data.find("\r\n");
        while (pos != std::string::npos)
        {
            data.replace(pos, 2, "\n");
            pos = data.find("\r\n");
        }

        return data;
    }

    static::std::string GetTempPathName()
    {
        static std::string tempPath;
        if (tempPath.empty())
        {
            std::stringstream ss;
            ss << std::filesystem::temp_directory_path().string() << "D2CE";
            tempPath = ss.str();
            std::filesystem::create_directories(tempPath);
        }

        return tempPath;
    }

    static std::string GetChar96TempPathName()
    {
        static std::string char96TempPath;
        if (char96TempPath.empty())
        {
            std::stringstream ss;
            ss << GetTempPathName() << "\\char\\96";
            char96TempPath = ss.str();
            std::filesystem::create_directories(char96TempPath);
        }

        return char96TempPath;
    }

    static std::string GetChar97TempPathName()
    {
        static std::string char96TempPath;
        if (char96TempPath.empty())
        {
            std::stringstream ss;
            ss << GetTempPathName() << "\\char\\97";
            char96TempPath = ss.str();
            std::filesystem::create_directories(char96TempPath);
        }

        return char96TempPath;
    }

    static std::string GetPathName()
    {
        static std::string modulePathName;
        if (modulePathName.empty())
        {
            HMODULE hm = NULL;
            if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                (LPSTR)&GetPathName, &hm) == 0)
            {
                return "";
            }

            char cstr[2048];
            DWORD dw = ::GetModuleFileNameA(hm, cstr, 2048);
            if (dw != 0)
            {
                // Remove the DLL's name from the path.
                modulePathName = cstr;
                size_t index = modulePathName.find_last_of("\\");
                if (index != modulePathName.npos)
                {
                    modulePathName = modulePathName.substr(0, index);
                }
            }
        }

        return modulePathName;
    }

    static std::string GetChar96PathName()
    {
        static std::string char96Path;
        if (char96Path.empty())
        {
            std::stringstream ss;
            ss << GetPathName() << "\\examples\\char\\96";
            char96Path = ss.str();
        }

        return char96Path;
    }

    static std::string GetChar97PathName()
    {
        static std::string char97Path;
        if (char97Path.empty())
        {
            std::stringstream ss;
            ss << GetPathName() << "\\examples\\char\\97";
            char97Path = ss.str();
        }

        return char97Path;
    }

    static bool LoadChar96File(const char* fileName, d2ce::Character& character, bool validateChecksum)
    {
        std::stringstream ss;
        ss << GetChar96PathName() << "\\" << fileName;
        return character.open(ss.str().c_str(), validateChecksum);
    }

    static bool LoadChar97File(const char* fileName, d2ce::Character& character, bool validateChecksum)
    {
        std::stringstream ss;
        ss << GetChar97PathName() << "\\" << fileName;
        return character.open(ss.str().c_str(), validateChecksum);
    }

    static bool LoadChar96TempFile(const char* fileName, d2ce::Character& character, bool validateChecksum)
    {
        std::string tempFile;
        {
            std::stringstream ss;
            ss << GetChar96TempPathName() << "\\" << fileName;
            tempFile = ss.str();
        }

        std::string origFile;
        {
            std::stringstream ss;
            ss << GetChar96PathName() << "\\" << fileName;
            origFile = ss.str();
        }
        std::filesystem::copy_file(origFile, tempFile, std::filesystem::copy_options::update_existing);
        std::filesystem::permissions(tempFile, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
        return character.open(tempFile.c_str(), validateChecksum);
    }

    static bool LoadChar97TempFile(const char* fileName, d2ce::Character& character, bool validateChecksum)
    {
        std::string tempFile;
        {
            std::stringstream ss;
            ss << GetChar97TempPathName() << "\\" << fileName;
            tempFile = ss.str();
        }

        std::string origFile;
        {
            std::stringstream ss;
            ss << GetChar97PathName() << "\\" << fileName;
            origFile = ss.str();
        }
        std::filesystem::copy_file(origFile, tempFile, std::filesystem::copy_options::update_existing);
        std::filesystem::permissions(tempFile, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
        return character.open(tempFile.c_str(), validateChecksum);
    }

    static std::string GetChar96ExpectedJsonOutput(const d2ce::Character& character)
    {
        std::stringstream ss;
        ss << GetChar96PathName() << "\\" << character.getName() << ".json";

#ifdef _MSC_VER
        std::ifstream expectedFile(ss.str(), std::ios::binary);
#else
        std::ifstream expectedFile(expectedPath, std::ios::binary);
#endif
        Assert::IsTrue(expectedFile.is_open() && expectedFile.good() && !expectedFile.eof()); // No fail, bad or EOF.
        std::string text((std::istreambuf_iterator<char>(expectedFile)), std::istreambuf_iterator<char>());
        return ConvertNewLines(text);
    }

    static std::string GetChar97ExpectedJsonOutput(const d2ce::Character& character)
    {
        std::stringstream ss;
        ss << GetChar97PathName() << "\\" << character.getName() << ".json";

#ifdef _MSC_VER
        std::ifstream expectedFile(ss.str(), std::ios::binary);
#else
        std::ifstream expectedFile(expectedPath, std::ios::binary);
#endif
        Assert::IsTrue(expectedFile.is_open() && expectedFile.good() && !expectedFile.eof()); // No fail, bad or EOF.
        std::string text((std::istreambuf_iterator<char>(expectedFile)), std::istreambuf_iterator<char>());
        return ConvertNewLines(text);
    }


    TEST_CLASS(D2EditorTests)
    {
    public:

        TEST_CLASS_CLEANUP(cleanUp)
        {
            std::filesystem::remove_all(GetTempPathName());
        }

		TEST_METHOD(TestOpen1)
		{
			d2ce::Character character;
            Assert::IsTrue(LoadChar96File("Merlina.d2s", character, true));
		}

        TEST_METHOD(TestOpen2)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadChar96File("Walter.d2s", character, true));
        }

        TEST_METHOD(TestOpen3)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadChar96File("WhirlWind.d2s", character, true));
        }

        TEST_METHOD(TestOpen4)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadChar97File("Scroll_Test.d2s", character, true));
        }

        TEST_METHOD(TestJsonExport1)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadChar96File("Merlina.d2s", character, true));
            auto json = character.asJson();
            Assert::AreEqual(ConvertNewLines(json), GetChar96ExpectedJsonOutput(character));
        }

        TEST_METHOD(TestJsonExport2)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadChar96File("Walter.d2s", character, true));
            auto json = character.asJson();
            Assert::AreEqual(ConvertNewLines(json), GetChar96ExpectedJsonOutput(character));
        }

        TEST_METHOD(TestJsonExport3)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadChar96File("WhirlWind.d2s", character, true));
            auto json = character.asJson();
            Assert::AreEqual(ConvertNewLines(json), GetChar96ExpectedJsonOutput(character));
        }

        TEST_METHOD(TestJsonExport4)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadChar97File("Scroll_Test.d2s", character, true));
            auto json = character.asJson();
            Assert::AreEqual(ConvertNewLines(json), GetChar97ExpectedJsonOutput(character));
        }

        TEST_METHOD(TestJsonTestComplexChange)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadChar96TempFile("Merlina.d2s", character, true));
            d2ce::BasicStats bs;
            character.fillBasicStats(bs);

            std::string strNewText = "Merlina_Two";
            std::string expectedTempPath;
            {
                std::stringstream ss;
                ss << GetChar96TempPathName() << "\\" << strNewText << ".d2s";
                expectedTempPath = ss.str();
                std::filesystem::remove(expectedTempPath);
            }

            std::memset(bs.Name, 0, sizeof(bs.Name));
            strcpy_s(bs.Name, strNewText.length() + 1, strNewText.c_str());
            character.updateBasicStats(bs);
            Assert::IsTrue(character.save());
            Assert::AreEqual(expectedTempPath.c_str(), character.getPathName());

            // Change some items
            character.maxDurabilityAllItems();
            character.fillAllStackables();
            character.upgradePotions();
            auto json = character.asJson();
            Assert::AreEqual(json, GetChar96ExpectedJsonOutput(character));
        }
	};
}
