// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"

#include "../command.hpp"


namespace pensar_digital
{
	using namespace pensar_digital::unit_test;
	namespace cpplib
	{
		inline static int value = 0;

		class IncCmd : public Command
		{
			public:
				using Ptr = std::shared_ptr<IncCmd>;
				IncCmd(const Id aid = NULL_ID) : Command(aid) { }
				~IncCmd() = default;

				using Factory = pd::Factory<IncCmd, Id>;
				inline static Factory mfactory = { 3, 10, NULL_ID }; //!< Member variable "factory"

				inline static typename Factory::P get(Id aid = NULL_ID) noexcept
				{
					return mfactory.get(aid);
				}
				
				// Meta information.
				inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("IncCmd"), 2, 1, 1 };
				inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

				virtual bool initialize(const Id id) noexcept
				{

					this->set_id(id == NULL_ID ? this->mgenerator.get_id() : id);

					return true;
				}
				Command::Ptr clone() const noexcept override { return pd::clone<IncCmd>(*this, id()); }
				inline void _run() { ++value; }
				inline void _undo() const { --value; }
		};

		class DecCmd : public Command
		{
			public:
				using Ptr = std::shared_ptr<DecCmd>;
				DecCmd(const Id aid = NULL_ID) : Command(aid) { }
				~DecCmd() = default;

				using Factory = pd::Factory<DecCmd, Id>;
				inline static Factory mfactory = { 3, 10, NULL_ID }; //!< Member variable "factory"

				inline static typename Factory::P get(Id aid = NULL_ID) noexcept
				{
					return mfactory.get(aid);
				}

				// Meta information.
				inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("DecCmd"), 2, 1, 1 };
				inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

				virtual bool initialize(const Id id) noexcept
				{

					this->set_id(id == NULL_ID ? this->mgenerator.get_id() : id);

					return true;
				}

				Command::Ptr clone() const noexcept override { return pd::clone<DecCmd>(*this, id()); }

				void _run() { --value; }
				void _undo() const { ++value; }
		};

		class IncFailCmd : public Command
		{
		public:
			using Ptr = std::shared_ptr<IncFailCmd>;
			IncFailCmd(const Id aid = NULL_ID) : Command(aid) { }
			~IncFailCmd() = default;

			using Factory = pd::Factory<IncFailCmd, Id>;
			inline static Factory mfactory = { 3, 10, NULL_ID }; //!< Member variable "factory"

			inline static typename Factory::P get(Id aid = NULL_ID) noexcept
			{
				return mfactory.get(aid);
			}
			// Meta information.
			inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("IncFailCmd"), 2, 1, 1 };
			inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

			virtual bool initialize(const Id id) noexcept
			{

				this->set_id(id == NULL_ID ? this->mgenerator.get_id() : id);

				return true;
			}
			Command::Ptr clone() const noexcept override { return pd::clone<IncFailCmd>(*this, id()); }

			void _run() { throw "IncFailCmd.run () error."; }
			void _undo() const { --value; }
		};


		class DoubleCmd : public Command
		{
			public:
				using Ptr = std::shared_ptr<DoubleCmd>;
				DoubleCmd(const Id aid = NULL_ID) : Command(aid) { }
				~DoubleCmd() = default;

				using Factory = pd::Factory<DoubleCmd, Id>;
				inline static Factory mfactory = { 3, 10, NULL_ID }; //!< Member variable "factory"

				inline static typename Factory::P get(Id aid = NULL_ID) noexcept
				{
					return mfactory.get(aid);
				}

				// Meta information.
				inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("DoubleCmd"), 2, 1, 1 };
				inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

				virtual bool initialize(const Id id) noexcept
				{

					this->set_id(id == NULL_ID ? this->mgenerator.get_id() : id);

					return true;
				}
				Command::Ptr clone() const noexcept override { return pd::clone<DoubleCmd>(*this, id()); }
				void _run() { value *= 2; }
				void _undo() const { value /= 2; }
		};

		class DoubleFailCmd : public Command
		{
			public:
				using Ptr = std::shared_ptr<DoubleFailCmd>;
				DoubleFailCmd(const Id aid = NULL_ID) : Command(aid) { }
				~DoubleFailCmd() = default;

				using Factory = pd::Factory<DoubleFailCmd, Id>;
				inline static Factory mfactory = { 3, 10, NULL_ID }; //!< Member variable "factory"

				inline static typename Factory::P get(Id aid = NULL_ID) noexcept
				{
					return mfactory.get(aid);
				}

				// Meta information.
				inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("DoubleFailCmd"), 2, 1, 1 };
				inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

				virtual bool initialize(const Id id) noexcept
				{

					this->set_id(id == NULL_ID ? this->mgenerator.get_id() : id);

					return true;
				}

				Command::Ptr clone() const noexcept override { return pd::clone<DoubleFailCmd>(*this, id()); }
				void _run() { throw "Double errors."; }
				void _undo() const { value /= 2; }
		};

		TEST(Command, true)
		{
			value = 0;

			IncCmd inc;
			CHECK_EQ(int, value, 0, "0");

			inc.run();
			CHECK_EQ(int, value, 1, "1");

			inc.undo();
			CHECK_EQ(int, value, 0, "2");

			DecCmd dec;

			CHECK_EQ(int, value, 0, "3");

			dec.run();
			CHECK_EQ(int, value, -1, "4");

			dec.undo();
			CHECK_EQ(int, value, 0, "5");

			IncFailCmd inc_fail;
			try
			{
				inc_fail.run();
			}
			catch (...)
			{
				CHECK_EQ(int, value, 0, "6");
			}

		}
		TEST_END(Command)

		TEST(CommandClone, true)
			IncCmd Cmd;
			IncCmd Cmd2(1);
			CHECK_NOT_EQ(IncCmd, Cmd, Cmd2, W("0"));
			auto ClonedCmd = Cmd.clone();
			IncCmd* ClonedCmdPtr = (IncCmd*)ClonedCmd.get ();
			CHECK_EQ(IncCmd, Cmd, *ClonedCmdPtr, W("1"));
		TEST_END(CommandClone)

		TEST(CompositeCommand, true)
		{

			using Cmd = CompositeCommand;
			static_assert (Identifiable<Cmd>);
			Cmd cmd;

			value = 0;

			CHECK_EQ(int, value, 0, "0");

			cmd.run();
			CHECK_EQ(int, value, 0, "1");

			CompositeCommand cmd2;
			cmd2.add(new IncCmd);
			cmd2.add(new DecCmd);
			cmd2.add(new DecCmd);
			cmd2.add(new DecCmd);


			CHECK_EQ(int, value, 0, "2");

			cmd2.run();
			CHECK_EQ(int, value, -2, "3");

			cmd2.undo();
			CHECK_EQ(int, value, 0, "4");


			CompositeCommand cmd3;
			cmd3.add(new IncCmd);
			cmd3.add(new DecCmd);
			cmd3.add(new IncFailCmd);
			CHECK_EQ(int, value, 0, "5");

			try
			{
				cmd3.run();
			}
			catch (...)
			{
				CHECK_EQ(int, value, 0, "6");
			}

			CHECK_EQ(int, value, 0, "7");

			value = 0;
			cmd2.run();
			CHECK_EQ(int, value, -2, "3");

			cmd2.undo();
			CHECK_EQ(int, value, 0, "4");

		}
		TEST_END(CompositeCommand)

		TEST(CommandBinaryStreaming, true)
			using Cmd = IncCmd;

			Cmd cmd;
			Cmd cmd2;
			CHECK_NOT_EQ(Cmd, cmd, cmd2, W("0"));
			BinaryBuffer bb;
			cmd.write (bb);
			cmd2.read (bb);
			CHECK_EQ(Cmd, cmd, cmd2, W("1"));
		TEST_END(CommandBinaryStreaming)

		
		TEST (CompositeCommandClone, true)
				using Cmd = CompositeCommand;
				Cmd cmd;
				Cmd cmd2;
				CHECK_NOT_EQ(Cmd, cmd, cmd2, W("0"));
				Command::Ptr cmd3_base = cmd.clone();
				Cmd::Ptr cmd3 = std::dynamic_pointer_cast<Cmd>(cmd3_base);
				Cmd& cmd4 = *cmd3;
				CHECK_EQ(Cmd, cmd4, cmd, W("1"));
		TEST_END(CompositeCommandClone)


		TEST(CompositeCmdBinaryStreaming, true)
			using Cmd = CompositeCommand;

			Cmd cmd;
			cmd.add(new NullCommand());  // Add a concrete command
			Cmd cmd2;
			// cmd2 starts empty - will be populated by read()
			BinaryBuffer buffer;
			// Write CompositeCommand's ClassInfo first (for polymorphic dispatch)
			// then the rest via cmd.write()
			cmd.write(buffer);
			// Read: first read ClassInfo to identify type
			ClassInfo info;
			buffer.read(std::span<std::byte>((std::byte*)&info, sizeof(ClassInfo)));
			// Then read the rest
			cmd2.read(buffer);
			CHECK_EQ(Cmd, cmd2, cmd, W("1"));
		TEST_END(CompositeCmdBinaryStreaming)
	}
}