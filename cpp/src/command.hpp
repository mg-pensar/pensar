#ifndef CPP_COMMAND_HPP
#define CPP_COMMAND_HPP

#include <iostream>
#include <vector>
#include <array>
#include <memory>
#include <exception>
#include <algorithm>
#include <type_traits>
#include <tuple>
#include <concepts>

#include "constant.hpp" // Id
#include "s.hpp"
#include "object.hpp"
#include "concept.hpp" // Identifiable
#include "generator.hpp"
#include "factory.hpp"
#include "bool.hpp"
#include "clone_util.hpp"
#include "equal.hpp"
#include "binary_buffer.hpp"


namespace pensar_digital
{
    namespace cpplib
    {
        //using namespace std;
        template<typename T>
        concept CommandConcept = requires(T t)
        {
            { t.run() } -> std::same_as<void>;
        }&& Identifiable<T> && CloneableConcept<T>;

        template<class T>
        concept DerivedCommandConcept = requires(T t)
		{
			{ t._run() } -> std::same_as<void>;
		}&& CloneableConcept<T>;

        template<typename T>
        concept UndoableCommandConcept = CommandConcept<T> && requires(T t)
        {
            { t.undo() } -> std::same_as<void>;
        };

        template<class T>
		concept DerivedUndoableCommandConcept = DerivedCommandConcept<T> && UndoableCommandConcept<T>;

        namespace pd = pensar_digital::cpplib;
        class Command : public Object
        {
            private:
            /// \brief Class Object::Data is compliant with the TriviallyCopyable concept. 
            /// \see https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable  
            struct Data : public pd::Data
            {
                Bool mok = Bool::UNKNOWN;
                Data(const Bool ok = Bool::UNKNOWN) noexcept : mok (ok) {}
            };
            // Asserts Data is a trivially copyable type.
            static_assert(StdLayoutTriviallyCopyableNoPadding<Data>, "Data must be a trivially copyable, no paddingtype");
            Data mdata; //!< Member variable mdata contains the object data
            
            public:
            using Ptr = std::shared_ptr<Command>;

            inline const static Data NULL_DATA = { Bool::UNKNOWN };
            using DataType = Data;
            using Factory =  pd::Factory<Command, Id, typename Command::DataType>;

            // Meta information.
            inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("Command"), 2, 1, 1 };
            inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }


            using FactoryType = Factory;

            virtual const pd::Data* data() const noexcept { return &mdata; }
            virtual const BytePtr data_bytes() const noexcept { return (BytePtr)data(); }

            virtual size_t data_size() const noexcept { return sizeof(mdata); }
			virtual size_t size() const noexcept { return data_size() + sizeof(ClassInfo) + Object::SIZE; }
            
            using G = Generator<Command, Id>; //!< Generator alias.

            inline static constexpr size_t DATA_SIZE = sizeof(mdata);
            inline static constexpr size_t      SIZE = Object::SIZE + DATA_SIZE + sizeof(ClassInfo) + G::SIZE;

            protected:
            
            inline static G mgenerator = G();

            public:

            //inline static Factory mfactory = { 3, 10, NULL_ID, NULL_DATA }; //!< Member variable "factory"


            Command (const Id aid = NULL_ID, const Data& data = NULL_DATA) : Object(aid == NULL_ID ? mgenerator.get_id() : aid), mdata(data) {}

            // Move constructor
            Command (Command&&) = default;

            // Copy constructor
            Command (const Command&) = default;

            // Move assignment
            Command& operator= (Command&&) = default;

            // Copy assignment
            Command& operator= (const Command&) = default;

            //Destructor
            virtual ~Command () = default;

            // Implements initialize method from Initializable concept.
            virtual bool initialize (const Id id, const Data& data) noexcept
            {
                
                this->set_id(id == NULL_ID ? mgenerator.get_id() : id);
                mdata = data;

                return true;
            }

			virtual bool equals(const Object& o) const noexcept
			{
				const Command* pother = dynamic_cast<const Command*>(&o);
				if (pother == nullptr)
					return false;
				return equal<Command> (*this, *pother);
			}

            virtual BinaryBuffer& write(BinaryBuffer& bb) const noexcept
            {
                Object::write(bb);
                bb.write (INFO.bytes () );
                bb.write (std::span<const std::byte>((const std::byte*)&mdata, DATA_SIZE));
                return bb;
            }
            virtual BinaryBuffer& read(BinaryBuffer& bb) noexcept
            {
               // Read Object part
                Object::read(bb);

                // Read ClassInfo first and verify
                ClassInfo info;
                bb.read(std::span<std::byte>((std::byte*)&info, sizeof(ClassInfo)));
                if (info != INFO)
                {
                    LOG(W("ClassInfo mismatch in Command::read"));
                    return bb;
                }
                // Read data bytes - use explicit span to avoid virtual call to derived class
                return bb.read(std::span<std::byte>((std::byte*)&mdata, DATA_SIZE));
            }
            protected:

                virtual void _run() = 0;  // Pure virtual - Command is abstract
                virtual void _undo() const  {}
                    
            public:
				virtual Ptr clone() const noexcept = 0; //!< Pure virtual - derived classes must implement.
            virtual void run () 
            {
                _run();
                mdata.mok = true;
            }

            void undo() const 
            {
                if (mdata.mok) // If the command was executed successfully, undo it.
                {
                    _undo();
                }
            }

            bool ok() const { return mdata.mok; }

        };

        class CommandRegistry 
        {
            using Creator = std::function<Command*()>;
            inline static std::unordered_map<S, Creator> registry;
            
            public:
            template<typename T>
            static void register_type() 
            {
                registry[T::INFO.full_class_name()] = []() { return new T(); };
            }
            
            static Command* create(const ClassInfo& info) 
            {

                auto it = registry.find(info.full_class_name());
                if (it != registry.end()) return it->second();
                return nullptr;
            }
        };

        // NullCommand is a command that does nothing.
        class NullCommand : public Command
		{
            public:
				using Ptr = std::shared_ptr<NullCommand>;
                NullCommand () : Command(NULL_ID) { }
				~NullCommand() = default;
				void _run() override { }
				void _undo() const override { }
                inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("NullCommand"), 2, 1, 1 };
                inline virtual const ClassInfo* info_ptr() const noexcept override { return &INFO; }

				Command::Ptr clone() const noexcept override { return pd::clone<NullCommand>(*this); }

                virtual BinaryBuffer& write(BinaryBuffer& bb) const noexcept override
                {
                    // Write NullCommand's ClassInfo FIRST for type identification
                    bb.write(INFO.bytes());
                    // Then write base class data
                    Command::write(bb);
                    return bb;
                }

                virtual BinaryBuffer& read(BinaryBuffer& bb) noexcept override
                {
                    // ClassInfo was already read by caller for type dispatch
                    // Just read the base class data
                    Command::read(bb);
                    return bb;
                }
                 
            private:
                // Static initializer to register the type
                inline static const bool _registered = []() {
                    CommandRegistry::register_type<NullCommand>();
                    return true;
                }();
       };
       inline static const NullCommand NULL_CMD = NullCommand();

        static_assert (Identifiable<NullCommand>);
        static_assert (CommandConcept<NullCommand>);
        static_assert (DerivedCommandConcept<NullCommand>);
        static_assert (UndoableCommandConcept<NullCommand>);
        static_assert (DerivedUndoableCommandConcept<NullCommand>);

        // CompositeCommand is a command that contains a list of commands to be executed in a transactional way.
        // If one of the commands fails, the entire transaction is rolled back.
        // The CompositeCommand is a Command and complies with the CommandConcept so itself can be part of another CompositeCommand.
		// The CompositeCommand will take ownership of the commands added to it.
		class CompositeCommand : public Command
		{
            public:
    		inline static const size_t MAX_COMMANDS = 10;
            using Ptr = std::shared_ptr<CompositeCommand>;
            
            private:
            struct Data : public pd::Data
            {
                using CommandArray = std::array<Command*, MAX_COMMANDS>;
                CommandArray mcommands{}; //!< Member variable "commands" contains the list of commands to be executed.
                size_t mindex;

                void add(Command* cmd)
                {
                    if (mindex >= MAX_COMMANDS)
                    {
                        log_and_throw("CompositeCommand::Data::add(Command* cmd) : Maximum number of commands reached.");
                    }
                    mcommands[mindex++] = cmd; 
                }

                void free_commands()
                {
                    // Deallocates commands in the array.
                    for (size_t i = 0; i < mindex; ++i)
                    {
                        delete mcommands[i];
                    }
                }
            };

            static_assert(StdLayoutTriviallyCopyableNoPadding<Data>, "Data must be a standard layout and trivially copyable type");
            Data mdata; //!< Member variable mdata contains the object data.

            public:
            inline const static Data NULL_DATA = { {}, 0 };
            using DataType = Data;
            using Factory = pd::Factory<CompositeCommand, Id>;
            using Int = int_fast8_t;
            inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("CompositeCommand"), 2, 1, 1 };
            inline virtual const ClassInfo* info_ptr() const noexcept override { return &INFO; }

            using FactoryType = Factory;

            virtual const pd::Data* data() const noexcept override { return &mdata; }

            inline static constexpr size_t DATA_SIZE = sizeof(mdata);

            virtual size_t data_size() const noexcept override { return sizeof(mdata); }
            virtual size_t size() const noexcept override 
            {
                size_t size = sizeof(mdata.mindex) + sizeof(ClassInfo);
                for (Int i = 0; i < mdata.mindex; ++i)
                {
                    size += mdata.mcommands[i]->size();
                }
                return size;
            }

            CompositeCommand(const Id aid = NULL_ID) : Command(aid), mdata (NULL_DATA)
            {
                
            }  

            ~CompositeCommand()
            {
                mdata.free_commands();
            }
        
            private:
            inline static Factory mfactory = { 3, 10, NULL_ID };

            public:

            // Implements initialize method from Initializable concept.
            virtual bool initialize (const Id id) noexcept
            {

                this->set_id (id == NULL_ID ? this->mgenerator.get_id() : id);

                return true;
            }

            inline static typename FactoryType::P get (const Id aid = NULL_ID) noexcept
            {
                return mfactory.get (aid);
            }
            void _run()
            {
                for (Int i = 0; i < mdata.mindex; ++i)
                {
                    mdata.mcommands[i]->run();
                }
            }
            void _undo() const
            {
                for (Int i = 0; i < mdata.mindex; ++i)
                {
                    mdata.mcommands[i]->undo();
                }
            }
        
            /// <summary>
            /// Adds the command to the composite.
            /// The CompositeCommand will take ownership of the command.
            /// </summary>
            /// <param name="cmd">Pointer to the command to be added.</param>
            void add (Command* cmd) { mdata.add (cmd); }

            Command::Ptr clone () const noexcept override
            {
                // Create a new CompositeCommand instance using Ptr and make_shared with the current id and mdata.
                CompositeCommand* cc_ptr =  new CompositeCommand(id());
                cc_ptr->mdata.mindex = mdata.mindex;

                // Clone all commands and add to the newly created composite.
                for (size_t i = 0; i < mdata.mindex; ++i)
                {
                    cc_ptr->add((Command*)mdata.mcommands[i]->clone().get ());
                }

                return Ptr(cc_ptr);
            }

            virtual bool equals(const Object& o) const noexcept
            {
                const CompositeCommand* pother = dynamic_cast<const CompositeCommand*>(&o);
                if (pother == nullptr)
                    return false;
                if (o.id () != id ())
                    return false;
                for (size_t i = 0; i < mdata.mindex; ++i)
                {
                    if (!mdata.mcommands[i]->equals(*pother->mdata.mcommands[i]))
                        return false;
                }
                return true;
            }

            virtual BinaryBuffer& write(BinaryBuffer& bb) const noexcept override
            {
                // Write CompositeCommand's ClassInfo FIRST for type identification
                bb.write(INFO.bytes());
                // Then write base class data
                Command::write(bb);
                // Write command count
                bb.write(mdata.mindex);
                // Write each command polymorphically
                for (size_t i = 0; i < mdata.mindex; ++i)
                {
                    mdata.mcommands[i]->write(bb);  // Virtual call writes type + data
                }
                return bb;
            }

            virtual BinaryBuffer& read(BinaryBuffer& bb) noexcept override
            {
                // ClassInfo was already read by caller for type dispatch (or verify it here)
                Command::read(bb);
                
                // Read command count
                bb.read(mdata.mindex);
                
                // Read each command - need a factory/registry
                for (size_t i = 0; i < mdata.mindex; ++i)
                {
                    // Read ClassInfo to determine type
                    ClassInfo cmd_info;
                    bb.read(std::span<std::byte>((std::byte*)&cmd_info, sizeof(ClassInfo)));
                    
                    // Create correct type based on info
                    Command* cmd = CommandRegistry::create(cmd_info);  // Factory pattern
                    if (cmd == nullptr)
                    {
                        LOG(W("Unknown command type in CompositeCommand::read: ") + cmd_info.full_class_name());
                        return bb;  // Can't continue - unknown type
                    }
                    cmd->read(bb);  // Finish reading rest of command data
                    mdata.mcommands[i] = cmd;
                }
                return bb;
            }        
        };  //  class CompositeCommand.
    }
}

#endif // CPP_COMMAND_HPP

