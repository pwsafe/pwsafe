#include "StringX.h"
#include "core.h"
#include "PolicyManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// class PolicyManager
///////////////////////////////////////////////////////////////////////////////////////////////////

const size_t PolicyManager::MAX_POLICIES = 255;

const stringT PolicyManager::GetDefaultPolicyName()
{
  stringT retval;
  LoadAString(retval, IDSC_DEFAULT_POLICY);
  return retval;
}

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
  return (m_Policies.find(StringX(name.c_str())) != m_Policies.end());
}

PWPolicy PolicyManager::GetDefaultPolicy() const
{
  return m_DefaultPolicy;
}

bool PolicyManager::IsDefaultPolicy(const stringT& name)
{
  return (name == GetDefaultPolicyName());
}

size_t PolicyManager::GetNumberOfPolicies() const
{
  return m_Policies.size();
}

bool PolicyManager::HasMaxPolicies() const
{
  return (m_Policies.size() >= PolicyManager::MAX_POLICIES);
}

bool PolicyManager::CanUndo() const
{
  return !m_UndoStack.empty();
}

bool PolicyManager::CanRedo() const
{
  return !m_RedoStack.empty();
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
