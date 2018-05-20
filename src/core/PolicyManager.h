#ifndef _POLICYMANAGER_H_
#define _POLICYMANAGER_H_

#include "core/Command.h"
#include "core/CommandInterface.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// The Policy Manager
///////////////////////////////////////////////////////////////////////////////////////////////////

class PolicyManager
{
private:
  PWPolicy                              m_DefaultPolicy;    /* The default policy, whose rules can only be modified. */
  PSWDPolicyMap                         m_Policies;         /* The collection of existing policies, on which the policy commands are operating on. */
  CommandInterface&                     m_CommandInterface; /* The interface to the Core. */
  std::vector<std::unique_ptr<Command>> m_UndoStack;        /* The lifo stack with undo commands. */
  std::vector<std::unique_ptr<Command>> m_RedoStack;        /* The lifo stack with redo commands. */
  
public:
  static const size_t  MAX_POLICIES;                        /* There are only 255 policy names allowed, because only 2 hex digits are used */
  static const stringT GetDefaultPolicyName();              /* The name of the default policy. */
  
  PolicyManager(CommandInterface& commandInterface);
  ~PolicyManager();
  
  /**
   * Creates a policy add command of type PolicyCommandAdd, 
   * executes the command and puts it on the Undo Stack.
   */
  void PolicyAdded(const stringT& name, const PWPolicy& policy);
  
  /**
   * Creates a policy remove command of type PolicyCommandRemove, 
   * executes the command and puts it on the Undo Stack.
   */
  void PolicyRemoved(const stringT& name);
  
  /**
   * Creates a policy modify command of type PolicyCommandModify, 
   * executes the command and puts it on the Undo Stack.
   */
  void PolicyModified(const stringT& name, const PWPolicy& original, const PWPolicy& modified);
  
  /**
   * Creates a policy rename command of type PolicyCommandRename, 
   * executes the command and puts it on the Undo Stack.
   */
  void PolicyRenamed(const stringT& oldName, const stringT& newName, const PWPolicy& original, const PWPolicy& modified);
  
  /**
   * Provides the internal collection of policies.
   */
  PSWDPolicyMap GetPolicies() const;
  
  /**
   * Provides a policy with the given name from the internal collection of policies.
   * 
   * If the requested policy doesn't exists the default policy will be provided.
   */
  PWPolicy GetPolicy(const stringT& name) const;
  
  /**
   * Provides the information whether a policy with the given name exists
   * in the internal collection of policies.
   */
  bool HasPolicy(const stringT& name) const;

  /**
   * Provides the default from the internal collection of policies.
   */
  PWPolicy GetDefaultPolicy() const;
  
  /**
   * Compares the given policy name against the default policy name.
   */
  static bool IsDefaultPolicy(const stringT& name);
  
  /**
   * Provides the number of policies within the internal collection of policies.
   */
  size_t GetNumberOfPolicies() const;
  
  /**
   * Provides the information whether the maximum allowed amount of policy entries has been reached.
   */
  bool HasMaxPolicies() const;
  
  /**
   * Provides the information whether an undo can be executed, 
   * which is the case if a command exists on the Undo Stack.
   */
  bool CanUndo() const;
  
  /**
   * Provides the information whether an redo can be executed, 
   * which is the case if a command exists on the Redo Stack.
   */
  bool CanRedo() const;
  
  /**
   * Executes undo on the last command from the Undo Stack, if a command exists.
   */
  void Undo();
  
  /**
   * Executes redo on the last command from the Redo Stack, if a command exists.
   */
  void Redo();
};

#endif /* _POLICYMANAGER_H_ */
