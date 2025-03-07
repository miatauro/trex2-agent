/*********************************************************************
 * Software License Agreement (BSD License)
 * 
 *  Copyright (c) 2011, MBARI.
 *  All rights reserved.
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 * 
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the TREX Project nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
/** @file core.cc
 * @brief T-REX core extensions for europa
 * 
 * This file declares the core extensions used  by T-REX within europa. The code 
 * found here handle to inject these extensions for each TREX::europa::Assembly
 * created within an agent. The extensions includes :
 * @li New set of constraints of functions for NDDL
 * @li The filters used by the planner and synchronizer of an Assembly
 * @li The new flaw manager and handler used by the synchronizer to identify Internal 
 * timeline state at every execution tick
 * 
 * @author Frederic Py <fpy@mbari.org>
 * @ingroup europa
 */
#include <trex/europa/Assembly.hh>
#include <trex/europa/DeliberationFilter.hh>
#include <trex/europa/ModeConstraints.hh>
#include <trex/europa/TimeConstraints.hh>
#include <trex/europa/SynchronizationManager.hh>
#include <trex/europa/TrexThreatDecisionPoint.hh>

#include "trex/europa/bits/europa_helpers.hh"
#include "private/CurrentState.hh"

// include plasma header as system files in order to disable warnings
# define TREX_PP_SYSTEM_FILE <CFunctions.hh>
# include <trex/europa/bits/system_header.hh>
# define TREX_PP_SYSTEM_FILE <DataTypes.hh>
# include <trex/europa/bits/system_header.hh>
# define TREX_PP_SYSTEM_FILE <OpenConditionDecisionPoint.hh>
# include <trex/europa/bits/system_header.hh>
# define TREX_PP_SYSTEM_FILE <TokenVariable.hh>
# include <trex/europa/bits/system_header.hh>



namespace TREX {
  namespace europa {

    using namespace EUROPA;
    
    /** @brief isExternal NDLL function declaration
     *
     * Declare to europa the new function @c isExternal that checks if the 
     * current token is a T-REX External timeline.
     *
     * @author Frederic Py
     * @relates CheckExternal
     * @ingroup europa
     */
    DECLARE_FUNCTION_TYPE(CheckExternal, external, 
                          "isExternal", EUROPA::BoolDT, 1); 
    /** @brief isInternal NDLL function declaration
     *
     * Declare to europa the new function @c isInternal that checks if the 
     * current token is a T-REX Internal timeline.
     *
     * @author Frederic Py
     * @relates CheckInternal
     * @ingroup europa
     */
    DECLARE_FUNCTION_TYPE(CheckInternal, internal, 
                          "isInternal", EUROPA::BoolDT, 1); 
                          
    DECLARE_FUNCTION_TYPE(TickFromDate, to_tick, 
                          "tick_date", EUROPA::IntDT, 3);
    
    /** @brief TREX core extensions for europa
     * 
     * This classe defines all the basic extensions of europa that
     * provide the core functionalities for the europa reactor. And are
     * used by T_REEX to implement synchronization and execution or for
     * the nddl modeler to acces to information that resides on T-REX in
     * the model. These extensions include :
     * @li @c DeliberationScope The filter that limits europa to plan only in
     * the planning window of the reactor 
     * [current tick, current tick+latency+lookahed]
     * @li @c SynchronizationScope The filter used during suynchronization
     * to limit the planning problem to only the current tick
     * @li CheckInternal A constraint that can be used to check if a
     * europa object is a TREX Internal timeline
     * @li CheckExternal A constraint that can be used to check if a
     * europa object is a TREX External timeline.
     *
     * @author Frederic Py <fpy@mbari.org>
     * @ingroup europa
     */
    class CoreExtensions :public EuropaPlugin {
    public:
      /** @brief Register core extensions
       *
       * @param[in] assembly A trex Assembly
       *
       * Register all the europa core extensions used in the wuropa
       * solver to interract with @p assembly. These extensions are
       * extra solver extensions (such as the TrexThreatHandler, the
       * horizon filters for synchronization and planning and the
       * current state flaw). But also utility constraints (sucha as
       * isExternal, isInternal or tick_date)
       */
      void registerComponents(Assembly const &assembly);
      
    }; // TREX::europa::CoreExtensions


    class TrexActionSupportHeuristic : public EUROPA::SOLVERS::ActionSupportHeuristic {
    public:
      TrexActionSupportHeuristic(std::vector<EUROPA::TokenTypeId>& choices)
	: ActionSupportHeuristic(), m_choices(choices) {}
      ~TrexActionSupportHeuristic() {}
      void getSupportCandidates(const TokenTypeId target,
				std::vector<EUROPA::SOLVERS::SupportChoice>& supports) {
	using EUROPA::SOLVERS::SupportChoice;
	for(std::vector<EUROPA::TokenTypeId>::const_iterator it = m_choices.begin();
	    it != m_choices.end();
	    ++it) {
	  supports.push_back(SupportChoice(target, *it, 0)); //NOTE: I have no idea what the numeric value here should be ~MJI
	}
      }
    private:
      std::vector<EUROPA::TokenTypeId> m_choices;
    };

#ifdef EUROPA_HAVE_EFFECT
    typedef EUROPA::SOLVERS::SupportedOCDecisionPoint default_oc_dp;

#else
    typedef EUROPA::SOLVERS::OpenConditionDecisionPoint default_oc_dp;
#endif
    
    
    class TrexOpenConditionDP:public default_oc_dp {
    public:
      TrexOpenConditionDP(EUROPA::DbClientId const &client,
			  EUROPA::TokenId const &flawed,
			  EUROPA::TiXmlElement const &configData,
			  EUROPA::LabelStr const &explanation = "trex")
	:default_oc_dp(client, flawed, configData, explanation) {
        //flawed->incRefCount();
      }
      ~TrexOpenConditionDP() {
        //m_flawedToken->decRefCount();
      }

      // std::string toShortString() const {
      //     return "europa sucks";
      // }
      std::string toString() const {
      	// Just make it compact for now as the toStringdid carsh at times
      	return toShortString();
      }

    private:
      void handleInitialize() {
#ifdef EUROPA_HAVE_EFFECT
        // There's no easty way to tweak SupportedOCDecisionPoint after its
        // init. So I just copy pasted the code instead
        const EUROPA::StateDomain stateDomain(m_flawedToken->getState()->lastDomain());
        
        if(stateDomain.isMember(EUROPA::Token::MERGED)){
          std::vector<EUROPA::TokenId> compatibleTokens;
          
          m_flawedToken->getPlanDatabase()->getCompatibleTokens(
                                                                m_flawedToken,
                                                                compatibleTokens,
                                                                std::numeric_limits<unsigned int>::max(),
                                                                true);
          
          std::reverse(compatibleTokens.begin(), compatibleTokens.end());

          // comment from europa code :
          // TODO: if flawed token is a fact, make sure we only look at other facts
          // trex comment: the above is weird ....
          if (compatibleTokens.size() > 0)
            m_choices.push_back(new EUROPA::SOLVERS::MergeToken(m_client,m_flawedToken,compatibleTokens));
        }
        
        if(stateDomain.isMember(Token::ACTIVE)) {
          SchemaId schema = m_client->getSchema();
          // TODO: PSToken::getTokenType() should return token type, not token type name
          TokenTypeId tokenType = schema->getTokenType(m_flawedToken->getFullTokenType());
          std::vector<TokenTypeId> supportActionTypes = schema->getTypeSupporters(tokenType);
          // TODO: allow heuristic function to be passed as a parameter to SupportToken
	  //MJI TODO NOTE: there is a DefaultActionSupportHeuristic that may do something
	  //better than this.  Investigate.
          if (!m_flawedToken->isFact() &&
              !m_flawedToken->hasAttributes(PSTokenType::EFFECT) &&
              (supportActionTypes.size() > 0))
	    m_choices.push_back(new EUROPA::SOLVERS::SupportToken(m_client,m_flawedToken,
								  new TrexActionSupportHeuristic(supportActionTypes)));
            //m_choices.push_back(new EUROPA::SOLVERS::SupportToken(m_client,m_flawedToken,supportActionTypes));
          else
            m_choices.push_back(new EUROPA::SOLVERS::ActivateToken(m_client,m_flawedToken));
        }
        
        if(stateDomain.isMember(Token::REJECTED))
          m_choices.push_back(new EUROPA::SOLVERS::RejectToken(m_client,m_flawedToken));
        
        
#else // !EUROPA_HAVE_EFFECT
	default_oc_dp::handleInitialize();
	// invert the order or merging decisions
	std::reverse(m_compatibleTokens.begin(), 
		     m_compatibleTokens.end());
#endif // EUROPA_HAVE_EFFECT
      }
    }; // TREX::europa::TrexOpenConditionDP

# define TREX_OC_HANDLER TrexOpenConditionHandler


    void CoreExtensions::registerComponents(Assembly const &assembly) {
      // T-REX core extensions
      TREX_REGISTER_FLAW_FILTER(assembly, TREX::europa::DeliberationScope, 
      				TREX_DELIB_FILT); 
      TREX_REGISTER_FLAW_FILTER(assembly, TREX::europa::SynchronizationScope, 
      				TREX_SYNCH_FILT); 
      TREX_REGISTER_FLAW_MANAGER(assembly, TREX::europa::SynchronizationManager,
				 TREX_SYNCH_MGR);
      TREX_REGISTER_FLAW_HANDLER(assembly, TREX::europa::details::CurrentState::DecisionPoint,
				 TREX_SYNCH_HANDLER);
      TREX_REGISTER_FLAW_HANDLER(assembly, TREX::europa::TrexThreatDecisionPoint,
				 TREX_THREAT_HANDLER);
      TREX_REGISTER_FLAW_HANDLER(assembly, TREX::europa::TrexOpenConditionDP,
				 TREX_OC_HANDLER);
				 

      // The new flaw to be matched
      TREX_REGISTER_MATCH_FINDER(assembly, TREX::europa::details::UpdateMatchFinder, 
                                 TREX::europa::details::CurrentState::entityTypeName());    

      // T-REX constraints
      TREX_REGISTER_CONSTRAINT(assembly,TREX::europa::CheckExternal,
			       isExternal,trex);
      TREX_REGISTER_CONSTRAINT(assembly,TREX::europa::CheckInternal,
			       isInternal,trex); 
      TREX_REGISTER_CONSTRAINT(assembly,TREX::europa::TickFromDate, 
                               tick_date, trex);
      
      declareFunction(assembly, new CheckExternalFunction());
      declareFunction(assembly, new CheckInternalFunction());
      declareFunction(assembly, new TickFromDateFunction());
    }

    CoreExtensions trex_core;

  } // TREX::europa
} // TREX
