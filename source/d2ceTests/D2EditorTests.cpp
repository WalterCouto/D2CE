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

    static const std::filesystem::path& GetChar99TempPathName()
    {
        static std::filesystem::path char99TempPath;
        if (char99TempPath.empty())
        {
            char99TempPath = GetCharBaseTempPathName() / L"99";
            std::filesystem::create_directories(char99TempPath);
        }

        return char99TempPath;
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

        case d2ce::EnumCharVersion::v120:
            return GetChar98TempPathName();

        default:
            return GetChar99TempPathName();
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

    static const std::filesystem::path& GetChar99PathName()
    {
        static std::filesystem::path char99Path;
        if (char99Path.empty())
        {
            char99Path = GetCharBasePathName() / L"99";
        }

        return char99Path;
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

        case d2ce::EnumCharVersion::v120:
            return GetChar98PathName();

        default:
            return GetChar99PathName();
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

    void TestJsonOpenBase(const std::filesystem::path& fileName, d2ce::EnumCharVersion version, bool validateChecksum, bool serialized = false, bool bCheckJsonExport = true)
    {
        std::string expectedJson;
        if (bCheckJsonExport)
        {
            std::filesystem::path expectedPath = GetCharPathName(version) / fileName;
            expectedJson = GetJsonText(expectedPath);
        }

        d2ce::Character character;
        Assert::IsTrue(LoadCharFile(fileName, character, version, validateChecksum));

        if (bCheckJsonExport)
        {
            auto json = character.asJson(serialized, d2ce::Character::EnumCharSaveOp::NoSave);
            Assert::AreEqual(ConvertNewLines(json), expectedJson);
        }
    }

    void TestJsonOpenBaseNoOutputCheck(const std::filesystem::path& fileName, d2ce::EnumCharVersion version, bool validateChecksum, bool serialized = false)
    {
        TestJsonOpenBase(fileName, version, validateChecksum, serialized, false);
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

        TEST_METHOD(TestOpen37)
        {
            TestOpenBase("Walter.d2s", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestOpen38)
        {
            TestOpenBase("immun_charm.d2s", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestOpen39)
        {
            TestOpenBase("Konan.d2s", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestOpen40)
        {
            TestOpenBase("Blizzard.d2s", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestOpen41)
        {
            TestOpenBase("Fire.d2s", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestOpen42)
        {
            TestOpenBase("JavaZone.d2s", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestOpen43)
        {
            TestOpenBase("Loradiel.d2s", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestOpen44)
        {
            TestOpenBase("Rui.d2s", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestOpen45)
        {
            TestOpenBase("Spartacus.d2s", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestOpen46)
        {
            TestOpenBase("Charming.d2s", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestOpen47)
        {
            TestOpenBase("HCharming.d2s", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestOpen48)
        {
            TestOpenBase("Markell.d2s", d2ce::EnumCharVersion::v140, true);
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

        TEST_METHOD(TestJsonExport45)
        {
            TestJsonExportBase("Walter.d2s", d2ce::EnumCharVersion::v140, true, false);
        }

        TEST_METHOD(TestJsonExport46)
        {
            TestJsonExportBase("immun_charm.d2s", d2ce::EnumCharVersion::v140, true, false);
        }

        TEST_METHOD(TestJsonExport47)
        {
            TestJsonExportBase("Konan.d2s", d2ce::EnumCharVersion::v140, true, false);
        }

        TEST_METHOD(TestJsonExport48)
        {
            TestJsonExportBase("Blizzard.d2s", d2ce::EnumCharVersion::v140, true, false);
        }

        TEST_METHOD(TestJsonExport49)
        {
            TestJsonExportBase("Fire.d2s", d2ce::EnumCharVersion::v140, true, false);
        }

        TEST_METHOD(TestJsonExport50)
        {
            TestJsonExportBase("JavaZone.d2s", d2ce::EnumCharVersion::v140, true, false);
        }

        TEST_METHOD(TestJsonExport51)
        {
            TestJsonExportBase("Loradiel.d2s", d2ce::EnumCharVersion::v140, true, false);
        }

        TEST_METHOD(TestJsonExport52)
        {
            TestJsonExportBase("Rui.d2s", d2ce::EnumCharVersion::v140, true, false);
        }

        TEST_METHOD(TestJsonExport53)
        {
            TestJsonExportBase("Spartacus.d2s", d2ce::EnumCharVersion::v140, true, false);
        }

        TEST_METHOD(TestJsonExport54)
        {
            TestJsonExportBase("Charming.d2s", d2ce::EnumCharVersion::v140, true, false);
        }

        TEST_METHOD(TestJsonExport55)
        {
            TestJsonExportBase("HCharming.d2s", d2ce::EnumCharVersion::v140, true, false);
        }

        TEST_METHOD(TestJsonExport56)
        {
            TestJsonExportBase("Poison.d2s", d2ce::EnumCharVersion::v110, true, false);
        }

        TEST_METHOD(TestJsonExport57)
        {
            TestJsonExportBase("Markell.d2s", d2ce::EnumCharVersion::v140, true, false);
        }

        TEST_METHOD(TestJsonOpen01)
        {
            TestJsonOpenBaseNoOutputCheck("Merlina.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen02)
        {
            TestJsonOpenBaseNoOutputCheck("Walter.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen03)
        {
            TestJsonOpenBaseNoOutputCheck("WhirlWind.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen04)
        {
            TestJsonOpenBaseNoOutputCheck("Scroll_Test.json", d2ce::EnumCharVersion::v100R, true);
        }

        TEST_METHOD(TestJsonOpen05)
        {
            TestJsonOpenBaseNoOutputCheck("testtt.json", d2ce::EnumCharVersion::v107, true);
        }

        TEST_METHOD(TestJsonOpen06)
        {
            TestJsonOpenBase("fddfss.json", d2ce::EnumCharVersion::v108, true);
        }

        TEST_METHOD(TestJsonOpen07)
        {
            TestJsonOpenBaseNoOutputCheck("afa.json", d2ce::EnumCharVersion::v109, true);
        }

        TEST_METHOD(TestJsonOpen08)
        {
            TestJsonOpenBaseNoOutputCheck("sdfsf.json", d2ce::EnumCharVersion::v109, true);
        }

        TEST_METHOD(TestJsonOpen09)
        {
            TestJsonOpenBaseNoOutputCheck("MfBowzon.json", d2ce::EnumCharVersion::v100R, true);
        }

        TEST_METHOD(TestJsonOpen10)
        {
            TestJsonOpenBase("test.json", d2ce::EnumCharVersion::v100R, true);
        }

        TEST_METHOD(TestJsonOpen11)
        {
            TestJsonOpenBaseNoOutputCheck("WhirlWind_serialized.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen12)
        {
            TestJsonOpenBase("test_serialized.json", d2ce::EnumCharVersion::v100R, true, true);
        }

        TEST_METHOD(TestJsonOpen13)
        {
            TestJsonOpenBaseNoOutputCheck("NokkaSorc.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen14)
        {
            TestJsonOpenBaseNoOutputCheck("Complex.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen15)
        {
            TestJsonOpenBaseNoOutputCheck("DannyIsGreatII.json", d2ce::EnumCharVersion::v100R, true);
        }

        TEST_METHOD(TestJsonOpen16)
        {
            TestJsonOpenBaseNoOutputCheck("Fist_YoU.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen17)
        {
            TestJsonOpenBaseNoOutputCheck("FukK_U.json", d2ce::EnumCharVersion::v110, true);
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
            TestJsonOpenBaseNoOutputCheck("Amazona.json", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestJsonOpen23)
        {
            TestJsonOpenBaseNoOutputCheck("Amazonb.json", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestJsonOpen24)
        {
            TestJsonOpenBaseNoOutputCheck("Amazonc.json", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestJsonOpen25)
        {
            TestJsonOpenBaseNoOutputCheck("Amazond.json", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestJsonOpen26)
        {
            TestJsonOpenBaseNoOutputCheck("Amazone.json", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestJsonOpen27)
        {
            TestJsonOpenBaseNoOutputCheck("Amazonf.json", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestJsonOpen28)
        {
            TestJsonOpenBaseNoOutputCheck("Amazong.json", d2ce::EnumCharVersion::v100, true);
        }

        TEST_METHOD(TestJsonOpen29)
        {
            TestJsonOpenBaseNoOutputCheck("Barbariana.json", d2ce::EnumCharVersion::v100, true);
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
            TestJsonOpenBaseNoOutputCheck("Drui.json", d2ce::EnumCharVersion::v120, true);
        }

        TEST_METHOD(TestJsonOpen33)
        {
            TestJsonOpenBaseNoOutputCheck("Blizzard.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen34)
        {
            TestJsonOpenBaseNoOutputCheck("BowAma.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen35)
        {
            TestJsonOpenBaseNoOutputCheck("Element.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen36)
        {
            TestJsonOpenBaseNoOutputCheck("Fire.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen37)
        {
            TestJsonOpenBase("FireClaw.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen38)
        {
            TestJsonOpenBaseNoOutputCheck("Frenzy.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen39)
        {
            TestJsonOpenBaseNoOutputCheck("Hammer.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen40)
        {
            TestJsonOpenBaseNoOutputCheck("JavaZone.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen41)
        {
            TestJsonOpenBaseNoOutputCheck("Lightning.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen42)
        {
            TestJsonOpenBaseNoOutputCheck("Orb.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen43)
        {
            TestJsonOpenBaseNoOutputCheck("Smiter.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen44)
        {
            TestJsonOpenBaseNoOutputCheck("Summon.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen45)
        {
            TestJsonOpenBaseNoOutputCheck("TollWut.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen46)
        {
            TestJsonOpenBaseNoOutputCheck("Traps.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen47)
        {
            TestJsonOpenBaseNoOutputCheck("Walter.json", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestJsonOpen48)
        {
            TestJsonOpenBaseNoOutputCheck("immun_charm.json", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestJsonOpen49)
        {
            TestJsonOpenBaseNoOutputCheck("Konan.json", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestJsonOpen50)
        {
            TestJsonOpenBaseNoOutputCheck("Blizzard.json", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestJsonOpen51)
        {
            TestJsonOpenBaseNoOutputCheck("Fire.json", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestJsonOpen52)
        {
            TestJsonOpenBaseNoOutputCheck("JavaZone.json", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestJsonOpen53)
        {
            TestJsonOpenBaseNoOutputCheck("Loradiel.json", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestJsonOpen54)
        {
            TestJsonOpenBaseNoOutputCheck("Rui.json", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestJsonOpen55)
        {
            TestJsonOpenBaseNoOutputCheck("Spartacus.json", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestJsonOpen56)
        {
            TestJsonOpenBaseNoOutputCheck("Charming.json", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestJsonOpen57)
        {
            TestJsonOpenBaseNoOutputCheck("HCharming.json", d2ce::EnumCharVersion::v140, true);
        }

        TEST_METHOD(TestJsonOpen58)
        {
            TestJsonOpenBaseNoOutputCheck("Poison.json", d2ce::EnumCharVersion::v110, true);
        }

        TEST_METHOD(TestJsonOpen59)
        {
            TestJsonOpenBaseNoOutputCheck("Markell.json", d2ce::EnumCharVersion::v140, true);
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
