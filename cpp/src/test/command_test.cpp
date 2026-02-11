// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>

#include "../command.hpp"

namespace pensar_digital::cpplib
{
    inline static int cmd_test_value = 0;

    class IncCmd : public Command
    {
        public:
            using Ptr = std::shared_ptr<IncCmd>;
            IncCmd(const Id aid = NULL_ID) : Command(aid) { }
            ~IncCmd() = default;

            using Factory = pd::Factory<IncCmd, Id>;
            inline static Factory mfactory = { 3, 10, NULL_ID };

            inline static typename Factory::P get(Id aid = NULL_ID) noexcept
            {
                return mfactory.get(aid);
            }

            inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("IncCmd"), 2, 1, 1 };
            inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

            virtual bool initialize(const Id id) noexcept
            {
                this->set_id(id == NULL_ID ? this->mgenerator.get_id() : id);
                return true;
            }
            Command::Ptr clone() const noexcept override { return pd::clone<IncCmd>(*this, id()); }
            inline void _run() { ++cmd_test_value; }
            inline void _undo() const { --cmd_test_value; }
    };

    class DecCmd : public Command
    {
        public:
            using Ptr = std::shared_ptr<DecCmd>;
            DecCmd(const Id aid = NULL_ID) : Command(aid) { }
            ~DecCmd() = default;

            using Factory = pd::Factory<DecCmd, Id>;
            inline static Factory mfactory = { 3, 10, NULL_ID };

            inline static typename Factory::P get(Id aid = NULL_ID) noexcept
            {
                return mfactory.get(aid);
            }

            inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("DecCmd"), 2, 1, 1 };
            inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

            virtual bool initialize(const Id id) noexcept
            {
                this->set_id(id == NULL_ID ? this->mgenerator.get_id() : id);
                return true;
            }

            Command::Ptr clone() const noexcept override { return pd::clone<DecCmd>(*this, id()); }
            void _run() { --cmd_test_value; }
            void _undo() const { ++cmd_test_value; }
    };

    class IncFailCmd : public Command
    {
    public:
        using Ptr = std::shared_ptr<IncFailCmd>;
        IncFailCmd(const Id aid = NULL_ID) : Command(aid) { }
        ~IncFailCmd() = default;

        using Factory = pd::Factory<IncFailCmd, Id>;
        inline static Factory mfactory = { 3, 10, NULL_ID };

        inline static typename Factory::P get(Id aid = NULL_ID) noexcept
        {
            return mfactory.get(aid);
        }

        inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("IncFailCmd"), 2, 1, 1 };
        inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

        virtual bool initialize(const Id id) noexcept
        {
            this->set_id(id == NULL_ID ? this->mgenerator.get_id() : id);
            return true;
        }
        Command::Ptr clone() const noexcept override { return pd::clone<IncFailCmd>(*this, id()); }
        void _run() { throw "IncFailCmd.run () error."; }
        void _undo() const { --cmd_test_value; }
    };

    class DoubleCmd : public Command
    {
        public:
            using Ptr = std::shared_ptr<DoubleCmd>;
            DoubleCmd(const Id aid = NULL_ID) : Command(aid) { }
            ~DoubleCmd() = default;

            using Factory = pd::Factory<DoubleCmd, Id>;
            inline static Factory mfactory = { 3, 10, NULL_ID };

            inline static typename Factory::P get(Id aid = NULL_ID) noexcept
            {
                return mfactory.get(aid);
            }

            inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("DoubleCmd"), 2, 1, 1 };
            inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

            virtual bool initialize(const Id id) noexcept
            {
                this->set_id(id == NULL_ID ? this->mgenerator.get_id() : id);
                return true;
            }
            Command::Ptr clone() const noexcept override { return pd::clone<DoubleCmd>(*this, id()); }
            void _run() { cmd_test_value *= 2; }
            void _undo() const { cmd_test_value /= 2; }
    };

    class DoubleFailCmd : public Command
    {
        public:
            using Ptr = std::shared_ptr<DoubleFailCmd>;
            DoubleFailCmd(const Id aid = NULL_ID) : Command(aid) { }
            ~DoubleFailCmd() = default;

            using Factory = pd::Factory<DoubleFailCmd, Id>;
            inline static Factory mfactory = { 3, 10, NULL_ID };

            inline static typename Factory::P get(Id aid = NULL_ID) noexcept
            {
                return mfactory.get(aid);
            }

            inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("DoubleFailCmd"), 2, 1, 1 };
            inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

            virtual bool initialize(const Id id) noexcept
            {
                this->set_id(id == NULL_ID ? this->mgenerator.get_id() : id);
                return true;
            }

            Command::Ptr clone() const noexcept override { return pd::clone<DoubleFailCmd>(*this, id()); }
            void _run() { throw "Double errors."; }
            void _undo() const { cmd_test_value /= 2; }
    };

    TEST_CASE("Command", "[command]")
    {
        cmd_test_value = 0;

        IncCmd inc;
        INFO("0"); CHECK(cmd_test_value == 0);

        inc.run();
        INFO("1"); CHECK(cmd_test_value == 1);

        inc.undo();
        INFO("2"); CHECK(cmd_test_value == 0);

        DecCmd dec;
        INFO("3"); CHECK(cmd_test_value == 0);

        dec.run();
        INFO("4"); CHECK(cmd_test_value == -1);

        dec.undo();
        INFO("5"); CHECK(cmd_test_value == 0);

        IncFailCmd inc_fail;
        try
        {
            inc_fail.run();
        }
        catch (...)
        {
            INFO("6"); CHECK(cmd_test_value == 0);
        }
    }

    TEST_CASE("CommandClone", "[command]")
    {
        IncCmd Cmd;
        IncCmd Cmd2(1);
        INFO(W("0")); CHECK(Cmd != Cmd2);
        auto ClonedCmd = Cmd.clone();
        IncCmd* ClonedCmdPtr = (IncCmd*)ClonedCmd.get();
        INFO(W("1")); CHECK(Cmd == *ClonedCmdPtr);
    }

    TEST_CASE("CompositeCommand", "[command]")
    {
        using Cmd = CompositeCommand;
        static_assert(Identifiable<Cmd>);
        Cmd cmd;

        cmd_test_value = 0;

        INFO("0"); CHECK(cmd_test_value == 0);

        cmd.run();
        INFO("1"); CHECK(cmd_test_value == 0);

        CompositeCommand cmd2;
        cmd2.add(new IncCmd);
        cmd2.add(new DecCmd);
        cmd2.add(new DecCmd);
        cmd2.add(new DecCmd);

        INFO("2"); CHECK(cmd_test_value == 0);

        cmd2.run();
        INFO("3"); CHECK(cmd_test_value == -2);

        cmd2.undo();
        INFO("4"); CHECK(cmd_test_value == 0);

        CompositeCommand cmd3;
        cmd3.add(new IncCmd);
        cmd3.add(new DecCmd);
        cmd3.add(new IncFailCmd);
        INFO("5"); CHECK(cmd_test_value == 0);

        try
        {
            cmd3.run();
        }
        catch (...)
        {
            INFO("6"); CHECK(cmd_test_value == 0);
        }

        INFO("7"); CHECK(cmd_test_value == 0);

        cmd_test_value = 0;
        cmd2.run();
        INFO("3"); CHECK(cmd_test_value == -2);

        cmd2.undo();
        INFO("4"); CHECK(cmd_test_value == 0);
    }

    TEST_CASE("CommandBinaryStreaming", "[command]")
    {
        using Cmd = IncCmd;

        Cmd cmd;
        Cmd cmd2;
        INFO(W("0")); CHECK(cmd != cmd2);
        BinaryBuffer bb;
        cmd.write(bb);
        cmd2.read(bb);
        INFO(W("1")); CHECK(cmd == cmd2);
    }

    TEST_CASE("CompositeCommandClone", "[command]")
    {
        using Cmd = CompositeCommand;
        Cmd cmd;
        Cmd cmd2;
        INFO(W("0")); CHECK(cmd != cmd2);
        Command::Ptr cmd3_base = cmd.clone();
        Cmd::Ptr cmd3 = std::dynamic_pointer_cast<Cmd>(cmd3_base);
        Cmd& cmd4 = *cmd3;
        INFO(W("1")); CHECK(cmd4 == cmd);
    }

    TEST_CASE("CompositeCmdBinaryStreaming", "[command]")
    {
        using Cmd = CompositeCommand;

        Cmd cmd;
        cmd.add(new NullCommand());
        Cmd cmd2;
        BinaryBuffer buffer;
        cmd.write(buffer);
        ClassInfo info;
        buffer.read(std::span<std::byte>((std::byte*)&info, sizeof(ClassInfo)));
        cmd2.read(buffer);
        INFO(W("1")); CHECK(cmd2 == cmd);
    }
}
