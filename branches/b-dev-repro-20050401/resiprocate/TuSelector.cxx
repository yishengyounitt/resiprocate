#include "resiprocate/TuSelector.hxx"
#include "resiprocate/TransactionUser.hxx"
#include "resiprocate/TransactionUserMessage.hxx"
#include "resiprocate/SipStack.hxx"
#include "resiprocate/os/TimeLimitFifo.hxx"

using namespace resip;

TuSelector::TuSelector(TimeLimitFifo<Message>& fallBackFifo) :
   mFallBackFifo(fallBackFifo) ,
   mTuSelectorMode(false)
{
}

void
TuSelector::process(TransactionUserMessage* msg)
{
   switch (msg->type() )
   {
      case TransactionUserMessage::RequestShutdown:
         markShuttingDown(msg->tu);
         break;
      case TransactionUserMessage::RemoveTransactionUser:
         remove(msg->tu);
         break;
      default:
         assert(0);
         break;
   }
}

void
TuSelector::add(Message* msg, TimeLimitFifo<Message>::DepthUsage usage)
{
   if (msg->hasTransactionUser() && exists(msg->getTransactionUser()))
   {
      msg->getTransactionUser()->postToTransactionUser(msg, usage);
   }
   else
   {
      mFallBackFifo.add(msg, usage);
   }
}

bool
TuSelector::wouldAccept(TimeLimitFifo<Message>::DepthUsage usage) const
{
   if (mTuSelectorMode)
   {
      for(TuList::const_iterator it = mTuList.begin(); it != mTuList.end(); it++)
      {
         if (!it->shuttingDown && it->tu->wouldAccept(usage))
         {
            return true;
         }
      }
      return false;
   }
   else
   {
      return mFallBackFifo.wouldAccept(usage);
   }
}
      
unsigned int 
TuSelector::size() const      
{
   if (mTuSelectorMode)
   {
      unsigned int total=0;   
      for(TuList::const_iterator it = mTuList.begin(); it != mTuList.end(); it++)
      {
         total += it->tu->size();
      }
      return total;
   }
   else
   {
      return mFallBackFifo.size();
   }
}

void 
TuSelector::registerTransactionUser(TransactionUser& tu)
{
   mTuSelectorMode = true;
   mTuList.push_back(Item(&tu));
}

TransactionUser* 
TuSelector::selectTransactionUser(const SipMessage& msg)
{
   for(TuList::iterator it = mTuList.begin(); it != mTuList.end(); it++)
   {
      if (it->tu->isForMe(msg))
      {
         return it->tu;
      }
   }
   return 0;
}

void
TuSelector::markShuttingDown(TransactionUser* tu)
{
   for(TuList::iterator it = mTuList.begin(); it != mTuList.end(); it++)
   {
      if (it->tu == tu)
      {
         it->shuttingDown = true;
         return;
      }
   }
   assert(0);
}

void
TuSelector::remove(TransactionUser* tu)
{
   for(TuList::iterator it = mTuList.begin(); it != mTuList.end(); it++)
   {
      if (it->tu == tu)
      {
         TransactionUserMessage* done = new TransactionUserMessage(TransactionUserMessage::TransactionUserRemoved, tu);
         tu->postToTransactionUser(done, TimeLimitFifo<resip::Message>::InternalElement);
         mTuList.erase(it);
         return;
      }
   }
   assert(0);
}

bool
TuSelector::exists(TransactionUser* tu)
{
   for(TuList::iterator it = mTuList.begin(); it != mTuList.end(); it++)
   {
      if (it->tu == tu)
      {
         return true;
      }
   }
   return false;
}
