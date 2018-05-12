#include "os/typedefs.h"
#include "PolicyManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// class PolicyManager
///////////////////////////////////////////////////////////////////////////////////////////////////

const stringT PolicyManager::DEFAULT_POLICYNAME(_S("Default Policy"));

PolicyManager::PolicyManager(CommandInterface& commandInterface) : m_CommandInterface(commandInterface)
{
  m_Policies      = commandInterface.GetPasswordPolicies();
  m_DefaultPolicy = PWSprefs::GetInstance()->GetDefaultPolicy();
}

PolicyManager::~PolicyManager() = default;

void PolicyManager::PolicyAdded(const stringT& name, const PWPolicy& policy)
{
  auto command = std::unique_ptr<Command>(new PolicyCommandAdd(m_CommandInterface, m_Policies, name, policy));
  
  command->Execute();
  
  m_UndoStack.push_back(std::move(command));
}

void PolicyManager::PolicyRemoved(const stringT& name)
{
  if (HasPolicy(name)) {
    
    auto command = std::unique_ptr<Command>(new PolicyCommandRemove(m_CommandInterface, m_Policies, name, m_Policies[StringX(name.c_str())]));
    
    command->Execute();
    
    m_UndoStack.push_back(std::move(command));
  }
}

void PolicyManager::PolicyModified(const stringT& name, const PWPolicy& original, const PWPolicy& modified)
{
  std::unique_ptr<Command> command;
  
  if (IsDefaultPolicy(name)) {
    
    command = std::unique_ptr<Command>(
      new PolicyCommandModify<SinglePolicyCollector, PWPolicy>(
        m_CommandInterface, m_DefaultPolicy, 
        name, original, modified
      )
    );
  }
  else {
    
    command = std::unique_ptr<Command>(
      new PolicyCommandModify<MultiPolicyCollector, PSWDPolicyMap>(
        m_CommandInterface, m_Policies, 
        name, original, modified
      )
    );
  }
  
  command->Execute();
  
  m_UndoStack.push_back(std::move(command));
}

void PolicyManager::PolicyRenamed(const stringT& oldName, const stringT& newName, const PWPolicy& original, const PWPolicy& modified)
{
  auto command = std::unique_ptr<Command>(new PolicyCommandRename(m_CommandInterface, m_Policies, oldName, newName, original, modified));
  
  command->Execute();
  
  m_UndoStack.push_back(std::move(command));
}

PSWDPolicyMap PolicyManager::GetPolicies() const
{
  return m_Policies;
}

PWPolicy PolicyManager::GetPolicy(const stringT& name) const
{
  return (m_Policies.find(StringX(name.c_str())) == m_Policies.end()) ? GetDefaultPolicy() : m_Policies.at(StringX(name.c_str()));
}

bool PolicyManager::HasPolicy(const stringT& name) const
{
  return (m_Policies.find(StringX(name.c_str())) == m_Policies.end()) ? false : true;
}

PWPolicy PolicyManager::GetDefaultPolicy() const
{
  return m_DefaultPolicy;
}

bool PolicyManager::IsDefaultPolicy(const stringT& name)
{
  return (name == PolicyManager::DEFAULT_POLICYNAME) ? true : false;
}

size_t PolicyManager::GetNumberOfPolicies() const
{
  return m_Policies.size();
}

bool PolicyManager::CanUndo() const
{
  return m_UndoStack.empty() ? false : true;
}

bool PolicyManager::CanRedo() const
{
  return m_RedoStack.empty() ? false : true;
}

void PolicyManager::Undo()
{
  if (!m_UndoStack.empty()) {
    auto command = std::move(m_UndoStack.back());
    command->Undo();
    m_UndoStack.pop_back();
    m_RedoStack.push_back(std::move(command));
  }
}

void PolicyManager::Redo()
{
  if (!m_RedoStack.empty()) {
    auto command = std::move(m_RedoStack.back());
    command->Redo();
    m_RedoStack.pop_back();
    m_UndoStack.push_back(std::move(command));
  }
}
