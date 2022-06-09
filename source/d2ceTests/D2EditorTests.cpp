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

    static const std::filesystem::path& GetTempPathName()
    {
        static std::filesystem::path tempPath;
        if (tempPath.empty())
        {
            tempPath = std::filesystem::temp_directory_path() / L"D2CE";
            std::filesystem::create_directories(tempPath);
        }

        return tempPath;
    }

    static const std::filesystem::path& GetCharBaseTempPathName()
    {
        static std::filesystem::path charBaseTempPath;
        if (charBaseTempPath.empty())
        {
            charBaseTempPath = GetTempPathName() / L"char";
            std::filesystem::create_directories(charBaseTempPath);
        }

        return charBaseTempPath;
    }

    static const std::filesystem::path& GetChar71TempPathName()
    {
        static std::filesystem::path char71TempPath;
        if (char71TempPath.empty())
        {
            char71TempPath = GetCharBaseTempPathName() / L"71";
            std::filesystem::create_directories(char71TempPath);
        }

        return char71TempPath;
    }

    static const std::filesystem::path& GetChar87TempPathName()
    {
        static std::filesystem::path char87TempPath;
        if (char87TempPath.empty())
        {
            char87TempPath = GetCharBaseTempPathName() / L"87";
            std::filesystem::create_directories(char87TempPath);
        }

        return char87TempPath;
    }

    static const std::filesystem::path& GetChar89TempPathName()
    {
        static std::filesystem::path char89TempPath;
        if (char89TempPath.empty())
        {
            char89TempPath = GetCharBaseTempPathName() / L"89";
            std::filesystem::create_directories(char89TempPath);
        }

        return char89TempPath;
    }

    static const std::filesystem::path& GetChar92TempPathName()
    {
        static std::filesystem::path char92TempPath;
        if (char92TempPath.empty())
        {
            char92TempPath = GetCharBaseTempPathName() / L"92";
            std::filesystem::create_directories(char92TempPath);
        }

        return char92TempPath;
    }

    static const std::filesystem::path& GetChar96TempPathName()
    {
        static std::filesystem::path char96TempPath;
        if (char96TempPath.empty())
        {
            char96TempPath = GetCharBaseTempPathName() / L"96";
            std::filesystem::create_directories(char96TempPath);
        }

        return char96TempPath;
    }

    static const std::filesystem::path& GetChar97TempPathName()
    {
        static std::filesystem::path char97TempPath;
        if (char97TempPath.empty())
        {
            char97TempPath = GetCharBaseTempPathName() / L"97";
            std::filesystem::create_directories(char97TempPath);
        }

        return char97TempPath;
    }

    static const std::filesystem::path& GetChar98TempPathName()
    {
        static std::filesystem::path char98TempPath;
        if (char98TempPath.empty())
        {
            char98TempPath = GetCharBaseTempPathName() / L"98";
            std::filesystem::create_directories(char98TempPath);
        }

        return char98TempPath;
    }

    static const std::filesystem::path& GetCharTempPathName(d2ce::EnumCharVersion version)
    {
        switch (version)
        {
        case d2ce::EnumCharVersion::v100:
            return GetChar71TempPathName();

        case d2ce::EnumCharVersion::v107:
            return GetChar87TempPathName();

        case d2ce::EnumCharVersion::v108:
            return GetChar89TempPathName();

        case d2ce::EnumCharVersion::v109:
            return GetChar92TempPathName();

        case d2ce::EnumCharVersion::v110:
            return GetChar96TempPathName();

        case d2ce::EnumCharVersion::v100R:
            return GetChar97TempPathName();

        default:
            return GetChar98TempPathName();
        }
    }

    static const std::filesystem::path& GetPathName()
    {
        static std::filesystem::path modulePath;
        if (modulePath.empty())
        {
            std::wstring modulePathName;
            HMODULE hm = NULL;
            if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                (LPWSTR)&GetPathName, &hm) == 0)
            {
                return modulePath;
            }

            wchar_t cstr[2048];
            DWORD dw = ::GetModuleFileNameW(hm, cstr, 2048);
            if (dw != 0)
            {
                // Remove the DLL's name from the path.
                modulePathName = cstr;
                size_t index = modulePathName.find_last_of(L"\\");
                if (index != modulePathName.npos)
                {
                    modulePath = modulePathName.substr(0, index);

                }
            }
        }

        return modulePath;
    }

    static const std::filesystem::path& GetCharBasePathName()
    {
        static std::filesystem::path charBasePath;
        if (charBasePath.empty())
        {
            charBasePath = GetPathName() / L"examples" / L"char";
        }

        return charBasePath;
    }

    static const std::filesystem::path& GetChar71PathName()
    {
        static std::filesystem::path char71Path;
        if (char71Path.empty())
        {
            char71Path = GetCharBasePathName() / L"71";
        }

        return char71Path;
    }

    static const std::filesystem::path& GetChar87PathName()
    {
        static std::filesystem::path char87Path;
        if (char87Path.empty())
        {
            char87Path = GetCharBasePathName() / L"87";
        }

        return char87Path;
    }

    static const std::filesystem::path& GetChar89PathName()
    {
        static std::filesystem::path char89Path;
        if (char89Path.empty())
        {
            char89Path = GetCharBasePathName() / L"89";
        }

        return char89Path;
    }

    static const std::filesystem::path& GetChar92PathName()
    {
        static std::filesystem::path char92Path;
        if (char92Path.empty())
        {
            char92Path = GetCharBasePathName() / L"92";
        }

        return char92Path;
    }

    static const std::filesystem::path& GetChar96PathName()
    {
        static std::filesystem::path char96Path;
        if (char96Path.empty())
        {
            char96Path = GetCharBasePathName() / L"96";
        }

        return char96Path;
    }

    static const std::filesystem::path& GetChar97PathName()
    {
        static std::filesystem::path char97Path;
        if (char97Path.empty())
        {
            char97Path = GetCharBasePathName() / L"97";
        }

        return char97Path;
    }

    static const std::filesystem::path& GetChar98PathName()
    {
        static std::filesystem::path char98Path;
        if (char98Path.empty())
        {
            char98Path = GetCharBasePathName() / L"98";
        }

        return char98Path;
    }

    static const std::filesystem::path& GetCharPathName(d2ce::EnumCharVersion version)
    {
        switch (version)
        {
        case d2ce::EnumCharVersion::v100:
            return GetChar71PathName();

        case d2ce::EnumCharVersion::v107:
            return GetChar87PathName();

        case d2ce::EnumCharVersion::v108:
            return GetChar89PathName();

        case d2ce::EnumCharVersion::v109:
            return GetChar92PathName();

        case d2ce::EnumCharVersion::v110:
            return GetChar96PathName();

        case d2ce::EnumCharVersion::v100R:
            return GetChar97PathName();

        default:
            return GetChar98PathName();
        }
    }

    static bool LoadCharFile(const std::filesystem::path& fileName, d2ce::Character& character, d2ce::EnumCharVersion version, bool validateChecksum)
    {
        std::filesystem::path p = GetCharPathName(version) / fileName;
        return character.open(p, validateChecksum);
    }

    static bool LoadCharTempFile(const char* fileName, d2ce::Character& character, d2ce::EnumCharVersion version, bool validateChecksum)
    {
        switch (version)
        {
        case d2ce::EnumCharVersion::v100:
            validateChecksum = false; // checksum not used
            break;
        }

        std::filesystem::path tempFile = GetCharTempPathName(version) / fileName;
        std::filesystem::path origFile = GetCharPathName(version) / fileName;
        std::filesystem::copy_file(origFile, tempFile, std::filesystem::copy_options::update_existing);
        std::filesystem::permissions(tempFile, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
        return character.open(tempFile, validateChecksum);
    }

    static std::string GetJsonText(const std::filesystem::path& path)
    {
#ifdef _MSC_VER
        std::ifstream expectedFile(path, std::ios::binary);
#else
        std::ifstream expectedFile(path, std::ios::binary);
#endif
        Assert::IsTrue(expectedFile.is_open() && expectedFile.good() && !expectedFile.eof()); // No fail, bad or EOF.
        std::string text((std::istreambuf_iterator<char>(expectedFile)), std::istreambuf_iterator<char>());
        return ConvertNewLines(text);
    }

    static std::string GetCharExpectedJsonOutput(const d2ce::Character& character, d2ce::EnumCharVersion version, bool bSerializedFormat = false)
    {
        std::stringstream ss;
        ss << character.getName().data();
        if (bSerializedFormat)
        {
            ss << "_serialized";
        }
        ss << ".json";

        std::filesystem::path expectedPath = GetCharPathName(version) / std::filesystem::u8path(ss.str());
        return GetJsonText(expectedPath);
    }

    void TestOpenBase(const std::filesystem::path& fileName, d2ce::EnumCharVersion version, bool validateChecksum)
    {
        d2ce::Character character;
        Assert::IsTrue(LoadCharFile(fileName, character, version, validateChecksum));
    }

    void TestJsonExportBase(const std::filesystem::path& fileName, d2ce::EnumCharVersion version, bool validateChecksum, bool serialized = false)
    {
        d2ce::Character character;
        Assert::IsTrue(LoadCharFile(fileName, character, version, validateChecksum));
        auto json = character.asJson(serialized, d2ce::Character::EnumCharSaveOp::NoSave);
        Assert::AreEqual(ConvertNewLines(json), GetCharExpectedJsonOutput(character, version, serialized));
    }

    void TestJsonOpenBase(const std::filesystem::path& fileName, d2ce::EnumCharVersion version, bool validateChecksum, bool serialized = false)
    {
        std::filesystem::path expectedPath = GetCharPathName(version) / fileName;
        auto expectedJson = GetJsonText(expectedPath);
        d2ce::Character character;
        Assert::IsTrue(LoadCharFile(fileName, character, version, validateChecksum));
        auto json = character.asJson(serialized, d2ce::Character::EnumCharSaveOp::NoSave);
        Assert::AreEqual(ConvertNewLines(json), expectedJson);
    }

    TEST_CLASS(D2EditorTests)
    {
    public:

        TEST_CLASS_CLEANUP(cleanUp)
        {
            std::filesystem::remove_all(GetTempPathName());
        }

		TEST_METHOD(TestOpen01)
		{
            TestOpenBase("Merlina.d2s", d2ce::EnumCharVersion::v110, true);
		}

        TEST_METHOD(TestOpen02)
        {
            TestOpenBase("Walter.d2s", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestOpen03)
        {
            TestOpenBase("WhirlWind.d2s", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestOpen04)
        {
            TestOpenBase("Scroll_Test.d2s", d2ce::EnumCharVersion::v100R, true);
        }

        TEST_METHOD(TestOpen05)
        {
            TestOpenBase("testtt.d2s", d2ce::EnumCharVersion::v107, true);
        }

        TEST_METHOD(TestOpen06)
        {
            TestOpenBase("fddfss.d2s", d2ce::EnumCharVersion::v108, true);
        }

        TEST_METHOD(TestOpen07)
        {
            TestOpenBase("afa.d2s", d2ce::EnumCharVersion::v109, true);
        }

        TEST_METHOD(TestOpen08)
        {
            TestOpenBase("sdfsf.d2s", d2ce::EnumCharVersion::v109, true);
        }

        TEST_METHOD(TestOpen09)
        {
            TestOpenBase("MfBowzon.d2s", d2ce::EnumCharVersion::v100R, true);
        }

        TEST_METHOD(TestOpen10)
        {
            TestOpenBase("test.d2s", d2ce::EnumCharVersion::v100R, true);
        }

        TEST_METHOD(TestOpen11)
        {
            TestOpenBase("Fist_YoU.d2s", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestOpen12)
        {
            TestOpenBase("FukK_U.d2s", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestOpen13)
        {
            TestOpenBase("Amazon.d2s", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestOpen14)
        {
            TestOpenBase("Barbarian.d2s", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestOpen15)
        {
            TestOpenBase("Bow.d2s", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestOpen16)
        {
            TestOpenBase("Necromancer.d2s", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestOpen17)
        {
            TestOpenBase("Paladin.d2s", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestOpen18)
        {
            TestOpenBase("Sorceress.d2s", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestOpen19)
        {
            TestOpenBase("Spartacus.d2s", d2ce::EnumCharVersion::v100R, true);
        }

        TEST_METHOD(TestOpen20)
        {
            TestOpenBase("Spartacus_II.d2s", d2ce::EnumCharVersion::v100R, true);
        }

        TEST_METHOD(TestOpen21)
        {
            // This character has the realm data of size 128 bits
            TestOpenBase("Loradiel.d2s", d2ce::EnumCharVersion::v100R, true);
        }

        TEST_METHOD(TestOpen22)
        {
            TestOpenBase("Amazona.d2s", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestOpen23)
        {
            TestOpenBase("Amazonb.d2s", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestOpen24)
        {
            TestOpenBase("Amazonc.d2s", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestOpen25)
        {
            TestOpenBase("Amazond.d2s", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestOpen26)
        {
            TestOpenBase("Amazone.d2s", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestOpen27)
        {
            TestOpenBase("Amazonf.d2s", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestOpen28)
        {
            TestOpenBase("Amazong.d2s", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestOpen29)
        {
            TestOpenBase("Barbariana.d2s", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestOpen30)
        {
            TestOpenBase(L"任务完成信.d2s", d2ce::EnumCharVersion::v120, true);
        }

        TEST_METHOD(TestOpen31)
        {
            TestOpenBase("Amazon.d2s", d2ce::EnumCharVersion::v120, true);
        }

        TEST_METHOD(TestOpen32)
        {
            TestOpenBase("Drui.d2s", d2ce::EnumCharVersion::v120, true);
        }

        TEST_METHOD(TestOpen33)
        {
            TestOpenBase("Nicro.d2s", d2ce::EnumCharVersion::v120, true);
        }

        TEST_METHOD(TestOpen34)
        {
            TestOpenBase("Rose.d2s", d2ce::EnumCharVersion::v120, true);
        }

        TEST_METHOD(TestOpen35)
        {
            TestOpenBase("Rui.d2s", d2ce::EnumCharVersion::v120, true);
        }

        TEST_METHOD(TestOpen36)
        {
            TestOpenBase("Walter.d2s", d2ce::EnumCharVersion::v120, true);
        }
        TEST_METHOD(TestJsonExport01)
        {
            TestJsonExportBase("Merlina.d2s", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonExport02)
        {
            TestJsonExportBase("Walter.d2s", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonExport03)
        {
            TestJsonExportBase("WhirlWind.d2s", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonExport04)
        {
            TestJsonExportBase("Scroll_Test.d2s", d2ce::EnumCharVersion::v100R, true);
        }

        TEST_METHOD(TestJsonExport05)
        {
            TestJsonExportBase("testtt.d2s", d2ce::EnumCharVersion::v107, true);
        }

        TEST_METHOD(TestJsonExport06)
        {
            TestJsonExportBase("fddfss.d2s", d2ce::EnumCharVersion::v108, true);
        }

        TEST_METHOD(TestJsonExport07)
        {
            TestJsonExportBase("afa.d2s", d2ce::EnumCharVersion::v109, true);
        }

        TEST_METHOD(TestJsonExport08)
        {
            TestJsonExportBase("sdfsf.d2s", d2ce::EnumCharVersion::v109, true);
        }

        TEST_METHOD(TestJsonExport09)
        {
            TestJsonExportBase("MfBowzon.d2s", d2ce::EnumCharVersion::v100R, true);
        }

        TEST_METHOD(TestJsonExport10)
        {
            TestJsonExportBase("test.d2s", d2ce::EnumCharVersion::v100R, true);
        }

        TEST_METHOD(TestJsonExport11)
        {
            TestJsonExportBase("WhirlWind.d2s", d2ce::EnumCharVersion::v110, true, true);
        }

        TEST_METHOD(TestJsonExport12)
        {
            TestJsonExportBase("test.d2s", d2ce::EnumCharVersion::v100R, true, true);
        }

        TEST_METHOD(TestJsonExport13)
        {
            TestJsonExportBase("DannyIsGreat.d2s", d2ce::EnumCharVersion::v100R, true, true);
        }

        TEST_METHOD(TestJsonExport14)
        {
            TestJsonExportBase("Fist_YoU.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonExport15)
        {
            TestJsonExportBase("FukK_U.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonExport16)
        {
            TestJsonExportBase("Amazon.d2s", d2ce::EnumCharVersion::v100, true, false);
        }

        TEST_METHOD(TestJsonExport17)
        {
            TestJsonExportBase("Barbarian.d2s", d2ce::EnumCharVersion::v100, true, false);
        }

        TEST_METHOD(TestJsonExport18)
        {
            TestJsonExportBase("Bow.d2s", d2ce::EnumCharVersion::v100, true, false);
        }

        TEST_METHOD(TestJsonExport19)
        {
            TestJsonExportBase("Necromancer.d2s", d2ce::EnumCharVersion::v100, true, false);
        }

        TEST_METHOD(TestJsonExport20)
        {
            TestJsonExportBase("Paladin.d2s", d2ce::EnumCharVersion::v100, true, false);
        }

        TEST_METHOD(TestJsonExport21)
        {
            TestJsonExportBase("Sorceress.d2s", d2ce::EnumCharVersion::v100, true, false);
        }

        TEST_METHOD(TestJsonExport22)
        {
            TestJsonExportBase("Spartacus.d2s", d2ce::EnumCharVersion::v100R, true, false);
        }

        TEST_METHOD(TestJsonExport23)
        {
            // This character has the realm data of size 128 bits
            TestJsonExportBase("Loradiel.d2s", d2ce::EnumCharVersion::v100R, true, false);
        }

        TEST_METHOD(TestJsonExport24)
        {
            TestJsonExportBase(L"任务完成信.d2s", d2ce::EnumCharVersion::v120, true, false);
        }

        TEST_METHOD(TestJsonExport25)
        {
            TestJsonExportBase("Amazon.d2s", d2ce::EnumCharVersion::v120, true, false);
        }

        TEST_METHOD(TestJsonExport26)
        {
            TestJsonExportBase("Drui.d2s", d2ce::EnumCharVersion::v120, true, false);
        }

        TEST_METHOD(TestJsonExport27)
        {
            TestJsonExportBase("Nicro.d2s", d2ce::EnumCharVersion::v120, true, false);
        }

        TEST_METHOD(TestJsonExport28)
        {
            TestJsonExportBase("Rose.d2s", d2ce::EnumCharVersion::v120, true, false);
        }

        TEST_METHOD(TestJsonExport29)
        {
            TestJsonExportBase("Rui.d2s", d2ce::EnumCharVersion::v120, true, false);
        }

        TEST_METHOD(TestJsonExport30)
        {
            TestJsonExportBase("Walter.d2s", d2ce::EnumCharVersion::v120, true, false);
        }

        TEST_METHOD(TestJsonExport31)
        {
            TestJsonExportBase("Blizzard.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonExport32)
        {
            TestJsonExportBase("BowAma.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonExport33)
        {
            TestJsonExportBase("Element.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonExport34)
        {
            TestJsonExportBase("Fire.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonExport35)
        {
            TestJsonExportBase("FireClaw.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonExport36)
        {
            TestJsonExportBase("Frenzy.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonExport37)
        {
            TestJsonExportBase("Hammer.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonExport38)
        {
            TestJsonExportBase("JavaZone.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonExport39)
        {
            TestJsonExportBase("Lightning.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonExport40)
        {
            TestJsonExportBase("Orb.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonExport41)
        {
            TestJsonExportBase("Smiter.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonExport42)
        {
            TestJsonExportBase("Summon.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonExport43)
        {
            TestJsonExportBase("TollWut.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonExport44)
        {
            TestJsonExportBase("Traps.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonOpen01)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Merlina.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen02)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Walter.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen03)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("WhirlWind.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen04)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Scroll_Test.json", character, d2ce::EnumCharVersion::v100R, true));
        }

        TEST_METHOD(TestJsonOpen05)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("testtt.json", character, d2ce::EnumCharVersion::v107, true));
        }

        TEST_METHOD(TestJsonOpen06)
        {
            TestJsonOpenBase("fddfss.json", d2ce::EnumCharVersion::v108, true);
        }

        TEST_METHOD(TestJsonOpen07)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("afa.json", character, d2ce::EnumCharVersion::v109, true));
        }

        TEST_METHOD(TestJsonOpen08)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("sdfsf.json", character, d2ce::EnumCharVersion::v109, true));
        }

        TEST_METHOD(TestJsonOpen09)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("MfBowzon.json", character, d2ce::EnumCharVersion::v100R, true));
        }

        TEST_METHOD(TestJsonOpen10)
        {
            TestJsonOpenBase("test.json", d2ce::EnumCharVersion::v100R, true);
        }

        TEST_METHOD(TestJsonOpen11)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("WhirlWind_serialized.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen12)
        {
            TestJsonOpenBase("test_serialized.json", d2ce::EnumCharVersion::v100R, true, true);
        }

        TEST_METHOD(TestJsonOpen13)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("NokkaSorc.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen14)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Complex.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen15)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("DannyIsGreatII.json", character, d2ce::EnumCharVersion::v100R, true));
        }

        TEST_METHOD(TestJsonOpen16)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Fist_YoU.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen17)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("FukK_U.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen18)
        {
            TestJsonOpenBase("Necromancer.json", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestJsonOpen19)
        {
            TestJsonOpenBase("Bow.json", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestJsonOpen20)
        {
            TestJsonOpenBase("Spartacus.json", d2ce::EnumCharVersion::v100R, true);
        }

        TEST_METHOD(TestJsonOpen21)
        {
            TestJsonOpenBase("Loradiel.json", d2ce::EnumCharVersion::v100R, true);
        }

        TEST_METHOD(TestJsonOpen22)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Amazona.json", character, d2ce::EnumCharVersion::v100, true));
        }

        TEST_METHOD(TestJsonOpen23)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Amazonb.json", character, d2ce::EnumCharVersion::v100, true));
        }

        TEST_METHOD(TestJsonOpen24)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Amazonc.json", character, d2ce::EnumCharVersion::v100, true));
        }

        TEST_METHOD(TestJsonOpen25)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Amazond.json", character, d2ce::EnumCharVersion::v100, true));
        }

        TEST_METHOD(TestJsonOpen26)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Amazone.json", character, d2ce::EnumCharVersion::v100, true));
        }

        TEST_METHOD(TestJsonOpen27)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Amazonf.json", character, d2ce::EnumCharVersion::v100, true));
        }

        TEST_METHOD(TestJsonOpen28)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Amazong.json", character, d2ce::EnumCharVersion::v100, true));
        }

        TEST_METHOD(TestJsonOpen29)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Barbariana.json", character, d2ce::EnumCharVersion::v100, true));
        }

        TEST_METHOD(TestJsonOpen30)
        {
            TestJsonOpenBase(L"任务完成信.json", d2ce::EnumCharVersion::v120, true);
        }

        TEST_METHOD(TestJsonOpen31)
        {
            TestJsonOpenBase("Rui.json", d2ce::EnumCharVersion::v120, true);
        }

        TEST_METHOD(TestJsonOpen32)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Drui.json", character, d2ce::EnumCharVersion::v120, true));
        }

        TEST_METHOD(TestJsonOpen33)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Blizzard.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen34)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("BowAma.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen35)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Element.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen36)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Fire.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen37)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("FireClaw.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen38)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Frenzy.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen39)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Hammer.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen40)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("JavaZone.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen41)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Lightning.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen42)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Orb.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen43)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Smiter.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen44)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Summon.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen45)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("TollWut.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonOpen46)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharFile("Traps.json", character, d2ce::EnumCharVersion::v110, true));
        }

        TEST_METHOD(TestJsonTestComplexChange01)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharTempFile("Merlina.d2s", character, d2ce::EnumCharVersion::v110, true));
            d2ce::BasicStats bs;
            character.fillBasicStats(bs);

            std::string strNewText = "Merlina_Two";
            std::filesystem::path expectedTempPath = GetCharTempPathName(d2ce::EnumCharVersion::v110);
            {
                std::stringstream ss;
                ss << strNewText << ".d2s";
                expectedTempPath /= std::filesystem::u8path(ss.str());
                std::filesystem::remove(expectedTempPath);
            }

            bs.Name.fill(0);
            strcpy_s(bs.Name.data(), strNewText.length() + 1, strNewText.c_str());
            character.updateBasicStats(bs);
            Assert::IsTrue(character.save(false));
            Assert::AreEqual(expectedTempPath.wstring(), character.getPath().wstring());

            // Change some items
            character.maxDurabilityAllItems();
            character.fillAllStackables();
            character.upgradePotions();
            Assert::IsTrue(character.save(false));
            auto json = character.asJson(false, d2ce::Character::EnumCharSaveOp::NoSave);
            Assert::AreEqual(json, GetCharExpectedJsonOutput(character, d2ce::EnumCharVersion::v110));
        }

        TEST_METHOD(TestJsonTestComplexChange02)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharTempFile("DannyIsGreat.d2s", character, d2ce::EnumCharVersion::v100R, true));
            Assert::IsTrue(character.isGameComplete());
            d2ce::BasicStats bs;
            character.fillBasicStats(bs);

            std::string strNewText = "DannyIsGreatIII";
            std::filesystem::path expectedTempPath = GetCharTempPathName(d2ce::EnumCharVersion::v100R);
            {
                std::stringstream ss;
                ss << strNewText << ".d2s";
                expectedTempPath /= std::filesystem::u8path(ss.str());
                std::filesystem::remove(expectedTempPath);
            }

            bs.Name.fill(0);
            strcpy_s(bs.Name.data(), strNewText.length() + 1, strNewText.c_str());
            character.updateBasicStats(bs);
            Assert::IsTrue(character.save(false));
            Assert::AreEqual(expectedTempPath.wstring(), character.getPath().wstring());

            // Change some items
            character.setDifficultyComplete(d2ce::EnumDifficulty::Normal);
            Assert::IsTrue(character.save(false));
            Assert::IsTrue(character.isDifficultyComplete((d2ce::EnumDifficulty::Normal)));
            Assert::IsFalse(character.isGameComplete());
            auto json = character.asJson(false, d2ce::Character::EnumCharSaveOp::NoSave);
            Assert::AreEqual(json, GetCharExpectedJsonOutput(character, d2ce::EnumCharVersion::v100R));

            strNewText = "DannyIsGreatIV";
            expectedTempPath = GetCharTempPathName(d2ce::EnumCharVersion::v100R);
            {
                std::stringstream ss;
                ss << strNewText << ".d2s";
                expectedTempPath /= std::filesystem::u8path(ss.str());
                std::filesystem::remove(expectedTempPath);
            }

            bs.Name.fill(0);
            strcpy_s(bs.Name.data(), strNewText.length() + 1, strNewText.c_str());
            character.updateBasicStats(bs);
            Assert::IsTrue(character.save(false));
            Assert::AreEqual(expectedTempPath.wstring(), character.getPath().wstring());

            // Change some items
            character.setGameComplete();
            Assert::IsTrue(character.save(false));
            Assert::IsTrue(character.isGameComplete());
            json = character.asJson(false, d2ce::Character::EnumCharSaveOp::NoSave);
            Assert::AreEqual(json, GetCharExpectedJsonOutput(character, d2ce::EnumCharVersion::v100R));

            strNewText = "DannyIsGreatV";
            expectedTempPath = GetCharTempPathName(d2ce::EnumCharVersion::v100R);
            {
                std::stringstream ss;
                ss << strNewText << ".d2s";
                expectedTempPath /= std::filesystem::u8path(ss.str());
                std::filesystem::remove(expectedTempPath);
            }

            bs.Name.fill(0);
            strcpy_s(bs.Name.data(), strNewText.length() + 1, strNewText.c_str());
            character.updateBasicStats(bs);
            Assert::IsTrue(character.save(false));
            Assert::AreEqual(expectedTempPath.wstring(), character.getPath().wstring());

            // Change some items
            character.setNoDifficultyComplete();
            Assert::IsTrue(character.save(false));
            Assert::IsFalse(character.isDifficultyComplete((d2ce::EnumDifficulty::Normal)));
            Assert::IsFalse(character.isGameComplete());
            json = character.asJson(false, d2ce::Character::EnumCharSaveOp::NoSave);
            Assert::AreEqual(json, GetCharExpectedJsonOutput(character, d2ce::EnumCharVersion::v100R));
        }

        TEST_METHOD(TestJsonTestComplexChange03)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharTempFile("Walter.d2s", character, d2ce::EnumCharVersion::v100R, true));
            d2ce::BasicStats bs;
            character.fillBasicStats(bs);

            std::string strNewText = "Walter_gem";
            std::filesystem::path expectedTempPath = GetCharTempPathName(d2ce::EnumCharVersion::v100R);
            {
                std::stringstream ss;
                ss << strNewText << ".d2s";
                expectedTempPath /= std::filesystem::u8path(ss.str());
                std::filesystem::remove(expectedTempPath);
            }

            bs.Name.fill(0);
            strcpy_s(bs.Name.data(), strNewText.length() + 1, strNewText.c_str());
            character.updateBasicStats(bs);
            Assert::IsTrue(character.save(false));
            Assert::AreEqual(expectedTempPath.wstring(), character.getPath().wstring());

            // Upgrade Gems
            Assert::AreEqual(character.upgradeGems(), size_t(2));
            Assert::IsTrue(character.save(false));
            auto json = character.asJson(false, d2ce::Character::EnumCharSaveOp::NoSave);
            Assert::AreEqual(json, GetCharExpectedJsonOutput(character, d2ce::EnumCharVersion::v100R));
        }

        TEST_METHOD(TestJsonTestComplexChange04)
        {
            d2ce::Character character;
            Assert::IsTrue(LoadCharTempFile("Spartacus.d2s", character, d2ce::EnumCharVersion::v100R, true));
            d2ce::BasicStats bs;
            character.fillBasicStats(bs);

            std::string strNewText = "Spartacus_II";
            std::filesystem::path expectedTempPath = GetCharTempPathName(d2ce::EnumCharVersion::v100R);
            {
                std::stringstream ss;
                ss << strNewText << ".d2s";
                expectedTempPath /= std::filesystem::u8path(ss.str());
                std::filesystem::remove(expectedTempPath);
            }

            bs.Name.fill(0);
            strcpy_s(bs.Name.data(), strNewText.length() + 1, strNewText.c_str());
            character.updateBasicStats(bs);
            Assert::IsTrue(character.save(false));
            Assert::AreEqual(expectedTempPath.wstring(), character.getPath().wstring());

            // Upgrade potions
            Assert::AreEqual(character.upgradeRejuvenationPotions(), size_t(26));
            Assert::IsTrue(character.save(false));
            auto json = character.asJson(false, d2ce::Character::EnumCharSaveOp::NoSave);
            Assert::AreEqual(json, GetCharExpectedJsonOutput(character, d2ce::EnumCharVersion::v100R));
        }
	};
}
