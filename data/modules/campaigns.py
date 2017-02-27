import universe
import campaign_lib
from campaign_lib import *

campaignsloaders=[lambda:LoadMainCampaign()]
campaigns=[]

_loaded=False

def loadAll(cockpit):
    global _loaded
    _loaded=True
    for x in campaignsloaders:
        ret=x()
        if ret:
            campaigns.append(ret)

def getCampaigns():
    if not _loaded:
        # Loading happens in generate_dyn_universe.py
        # But sometimes, e.g. multiplayer, this is not loaded.
        loadAll(VS.getCurrentPlayer())
    return campaigns

### The Main Campaign

haulerdeal1={"intro":[("Hauler", "That was lucky, I thought I might have missed you!"),
                    ("Deucalion", "I think you've mistaken me for someone else."),
                    ("Hauler", "No, I'm certain - you're running the Llama that just got recertified."),
                    "Deucalion stares ambiguously at this vexacious interruption.",
                    ("Deucalion", "I'm not buying."),
                    ("Hauler", "No, no, no.  You misunderstand, I'm a freight merchant, my hauler has broken down and I need help delivering the rest of my shipment."),
                    ("Deucalion", "I've got my own problems right now, find someo--"),
                    ("Hauler", "Jenek, my name's Jenek.  Look, just two crates, just two systems.  Anything at all would be great."),
                    ("Deucalion", "I'm sor--"),
                    ("Jenek", "I can't pay you anything, but I'll be in your personal debt.  What do you say?")],
            "reject1":[("Jenek", "I understand.  Hope you get your problems sorted out.  If you change your mind, I can't see me leaving here anytime soon.")],
            "reconsider":[("Jenek", "You're back!  I wasn't expecting to see you again.  Have you reconsidered?")],
            "reject2":[("Deucalion", "No."),
                        ("Jenek", "Well then, this shipment isn't going anywhere anytime soon.  If you change your mind...")],
            "failure":[("Jenek", "You're back again, did it all go okay?"),
                        ("Deucalion", "There were problems."),
                        ("Jenek", "What?!?  You mean you didn't get it delivered?  That's gotta be the simplest cargo mission you could ever get, and you couldn't do it?  You're not getting any more jobs from me, I'll wait until the insurance comes through for the repair bill.  The cargo may not be on time, but at least this way I know it'll get there.")],
            "reminder":[("Jenek", "Jump to Stirling, jump to Bernards star, deliver crates to the arid planet.  Get going!")],
            "accept":[("Jenek", "Great, thank you.  There's a couple of crates of medical supplies that need to get delivered to the arid planet, Jacobs, in Bernards star.  Just jump to Stirling, then to Bernards star, and then land at the planet's spaceport!  I'll get the crates loaded now.")]
        }

haulerdeal2={"intro":[("Jenek", "Thanks for that, you really helped me out."),
                    ("Deucalion", "No problem."),
                    ("Jenek", "I really mean it, and I want to repay you.  I just don't have the finance to do it at the moment, but once the rest of this shipment has been delivered..."),
                    "Deucalion, observing the brief flash of inspiration in Jenek's eyes, acts to avoid the imminent sales onslaught.",
                    ("Deucalion", "I'll deliver the rest of the shipment for you."),
                    ("Jenek", "Look, I know that you've already helped me out, and I wouldn't ... What?"),
                    ("Deucalion", "I'll deliver the shipment for you, if you still want me to of course."),
                    "With a look of surprised bewilderment frozen on his face, Jenek momentarilly loses himself in his own thoughts.",
                    ("Jenek", "... You really mean it?")],
            "reject1":[("Jenek", "Oh ... Okay.")],
            "reconsider":[("Jenek", "Please, don't toy with me, goodwill seems to be in rather short supply these days.  If you want to help me just say, otherwise it might be safer for you if you hadn't met me.")],
            "reject2":[("Deucalion", "No."),
                        ("Jenek", "I can't really blame you, I probably would have made the same decision.  Space isn't as friendly around these parts since the war arrived.  But even though you don't want my shipment, take my advice ...  the choices you make today will forever affect your future. Oh, and watch out for Luviccio.")],
            "reminder":[("Jenek", "Just follow the directions I gave you, and sign over the shipment to Luviccio.")],
            "accept":[("Jenek", "Thank you so much.  I was beginning to think I wouldn't be able to get anyone to help.  You have no idea how much this means to me!"),
                        ("Deucalion","Where am I taking the shipment?"),
                        ("Jenek","I've managed to organise another merchant to finish shipping to all the individual recipients, so you just have to take it all to him.  His name is Luviccio, and you'll have to get him to sign off on the delivery personally.  You should be able to contact him on Serenity Mining Base.")]
        }

haulerreward=[("Jenek","Thank you so much for finishing that.  You have helped me more than you know."),
            ("Deucalion","It's okay, it wasn't too much of an effort on my part."),
            ("Jenek","That may be, but most people would not have done what you have.  You are a good man."),
            "Deucalion is a little surprised at how glad Jenek is, but doesn't think further of it.",
            ("Jenek","And, as I promised, I have a little something for you.  There are now 2000 more credits in your bank account.  I know it's not much, but it's all I can afford."),
            ("Deucalion","I really wasn't expect--"),
            ("Jenek","I know, but I promised, and I want to try and make up for what I may have just gotten you into."),
            ("Deucalion","What are you talking about?"),
            ("Jenek","Luviccio, the man you delivered the last set of crates to, thanks you for your work and wants to speak with you further.  As innocent as it sounds, he's not to be trusted.  If you decide to meet him, be careful, but I strongly recommend you don't go and see him at all.  Whatever choice you make now, thank you again for your help."),
            "Jenek finishes his drink and prepares to leave.  \"This little adventure,\" Deucalion thought, \"seems to be over.\""]

gangsterswindled=[("Patron", "Pardon the abruptness and the liberty of this conversation;  It is due to its exigency.  I am a Credit official in New Baikal.  I ask for your help.  I am in need of a foreign partner to assist in the transfer of a considerable amount of money.  What we require is someone who is reliable and trustworthy, who has a vision and who will be able to manage whatever business in which we shall inject venture capital.  We need someone who can share our dreams and ideals."),
                    ("Deucalion", "Pardon me but, who are you?"),
                    ("Patron","Please do not at this point misunderstand my intention, because understandably, we are not acqainted.  I do not even know if you will be reliable and trustworthy, but I believe that everything starts with a first step ... If you will excuse me."),
                    "The man turns to answer a communication.",
                    ("Patron", "Dear friend, you must forgive my rudeness, but I must go.  Thank you for your courtesies.")]

angryhauler=[("Jenek", "Why haven't you delivered the cargo yet?  The recipient is starting to get angry."),
            ("Deucalion","The cargo wont be getting delivered ... I lost it."),
            "Jenek's agitation level noticably increased.",
            ("Jenek","This isn't good for either of us, the man you were to deliver to has a reputation for extracting compensation for premature contract termination.  I hope you know how to take care of yourself .. you're going to need to."),
            "Deucalion felt it was best to leave the hauler alone, Jenek was already angry and stressed enough as it is."]

angrygangster=[("Patron", "You are the man who was to deliver my shipment."),
            "Taken aback by the abrupness of this statement, \"How did he know?\", Deucalion is lost for words",
            ("Deucalion","I ..."),
            ("Patron","It was not a question.  I know who you are, and I know what you were contracted for.  I know you broke that contract, and I know that there will be a price to pay.  What I do not know is what that price will be.  Do not attempt to leave this system however, or payment of this debt will be your life.")]

confed_tail_finance={"intro":[("Investigator","I thought you might want to talk to me.  I believe you've been the subject of some credit fraud."),
                            "Deucalion doesn't react, the investigator starts to look uncomfortable.  He might not be genuine, but he certainly isn't from around here.",
                            ("Deucalion","Who are you?"),
                            ("Investigator","I'm with the Credit Fraud division of Confed IntelSEC.  We've been aware of a particular individual perpetrating multiple offences in this system for a short time.  Unfortunately, most of the victims don't want to have anything to do with a Confederate investigation, much less one relating to credit fraud."),
                            ("Deucalion","I can't say that surprises me.  Here one learns to take care of himself, as the Confederate presence isn't, well ... overwhelming."),
                            ("Investigator","Resources are finite, and must be used where they would be most effective.  But no, I don't blame them.  Most, allowing for the occasional unscrupulous credit manager, will get their money back in the end ... and most will probably want to remain as small targets to the criminal organisations as possible.  So in the end there is little incentive for those happy with their situation."),
"\'Salesmen,\' Deucalion thinks, \'I must be wearing some kind of target.\'",
                            ("Deucalion","What do you want?"),
                            ("Investigator", "To make a difference.  To get a nice job in the inner sphere.  To settle down with a nice family ..."),
                            ("Deucalion","And of me?"),
                            ("Investigator","To help make a difference, to help rid this sector of another criminal.  Of course, such help wouldn't be unrewarded.  I would imagine with the perpetrator of these crimes officially dealt with, your creditors might be more willing to respond to your fraud claims ... and there're always discretionary funds to help finance any assistance leant to investigations.  There is though, only one question that really matters in these situations.  Are you the man for this job?")],
        "accept":[("Investigator","Sorry, I almost forgot ... my identification ..."),
                  ("Deucalion","Don't worry about that.  I don't know whether I'm the right man, but I'm willing to see."),
                  "Although the response wasn't really an accurate representation of how he felt about all of this, it was the only course of action, Deucalion could see, that had a chance of ending well.",
		          ("Investigator","Good.  What you have to do is not hard, it's actually really straight forward.  But it needs to be done well, or all of this is for nothing.  Your target is a message runner for the criminal organisation.  He is not of any interest, it is the message he is delivering.  You are to follow this target until he broadcasts this message.  Once the message has been recorded by your computer, you are done.  We'll take it from there."),
		          ("Deucalion", "Sounds easy enough."),
		          ("Investigator","It isn't.  You have to stay far enough away from the target so that he does not pick you up as a threat, and so that you do not noticably dampen his SPEC multiple.  Yet at the same time you must be close enough so that he doesn't lose you jumping from system to system.  If he notices you, or you lose him, the mission is failed, and the deal is off.  The target will be leaving this system fairly shortly, you'd better go now."),
		          ("Deucalion","What abo-"),
		          ("Investigator","Payment?  Come and see me after you complete the mission.")],
		"reject1":["Deucalion reconsiders this man.  He is certainly not the uncomfortable beurocrat he initially appeared.",
		          ("Deucalion","I think I'll sit this one out.")],
		"reconsider":[("Deucalion","If you're still interested, I may be willing to help."),
                        ("Investigator","Good.  If it helps make your decision, I can guarantee 50,000 credits as a reward if successful.  But you're going to have to decide now, as I do have another candidate who seems more enthusiastic.  What'll it be, will you help?")],
		"reject2":[("Deucalion","No, I think I'll let the other guy handle this one."),
                    ("Investigator","I'm sorry to see you go.")],
		"reminder":[("Investigator","You want to see my identification again?"),
                    ("Deucalion","What?  No!  Put that thing away."),
                    ("Investigator","Then you'd better get moving.  After all, the target isn't going to wait for you.")],
		"failure":[("Investigator","I made a mistake, I thought you were up to this."),
		          ("Deucalion","Things wer-"),
		          ("Investigator","Complicated?  Yes, I heard about what you were up to before we met.  If I'd known beforehand things may have turned out differently.  As it is, I'd advise you to leave this system and try and keep a low profile.")]
		}

confed_tail_insider=dict(confed_tail_finance)
confed_tail_insider["intro"]=[("Investigator","Ah, Deucalion isn't it?  Yes, you're the mysterious pilot I've been hearing so much about.  I need to talk to you about something, I believe you've recently had some dealings with the local criminal element?  The one headed by a man named Luviccio?"),
                            "Deucalion doesn't react, the investigator starts to look uncomfortable.  He might not be genuine, but he certainly isn't from around here.",
                            ("Deucalion","Who are you?"),
                            ("Investigator","I'm with the Credit Fraud division of Confed IntelSEC.  We've been aware of a particular individual perpetrating multiple offences in this system, mostly from the Plainfield Mining Base, for a short time.  Unfortunately, most of the people who've had contact with him don't want to have anything to do with a Confederate investigation, much less one relating to credit fraud."),
                            ("Deucalion","I can't say that surprises me.  Here one learns to take care of himself, as the Confederate presence isn't, well ... overwhelming."),
                            ("Investigator","Resources are finite, and must be used where they would be most effective.  But no, I don't blame them.  Most of the credit victims, allowing for the occasional unscrupulous credit manager, will get their money back in the end ... and most will probably want to remain as small targets to the criminal organisations as possible.  The others are almost always dead before I have the chance to speak to them, so overall the leads in this investigation are few and far between.  You on the other hand are a different matter, and are in a very unique position."),
                            ("Deucalion","I haven't been a victim?"),
                            ("Investigator", "Exactly, your involvement with this criminal has been of a different nature and, if the situation is what I believe it to be, you are already on Luviccio's bad side."),
                            ("Deucalion","So what do you want of me?"),
                            ("Investigator","To help make a difference, to help rid this sector of this criminal.  Of course, such help wouldn't be unrewarded, there're always discretionary funds to help finance any assistance leant to investigations.  There is though, only one question that really matters in these situations.  Are you the man for this job?")]

tailreward=[("Investigator","Well done, the mission was a complete success!  Luviccio has been arrested, his while organisation has been shut down, and the creditors are now legally compelled to repay those who have been subject to his credit fraud.  Although you wont get public recognition, my director thanks you for your help."),
            ("Deucalion","As much as I appreciate the gratitude, that's not why I did the mission."),
            ("Investigator","Yes, I know. You didn't really have much of a choice did you?  To make up for it, I have, as I said I would, arranged a little financial assistance for losses incurred during your assistance.  It has already been transferred to your account.  Good luck for the future.")]

gangstersellout=[("Patron","Good day sir. I think we have some business to discuss."),
                ("Deucalion","No we don't, I have nothing I want to talk to you about."),
                ("Patron","Nevertheless, you are still here!"),
                "As he speaks, several people get up from nearby tables, and surround Deucalion.",
                ("Deucalion","It appears I have little choice in the matter."),
                "Ignoring Deucalion's remarks, the man continues...",
                ("Patron","A little bird has told me that someone has been aiding an IntelSEC investigation.  This IntelSEC investigation puts in danger a great many important people, who do a great deal of important work."),
                "Deucalion attempts to interrupt, but a sudden exertion of pressure on his back suggests that it would be a good idea not to.",
                ("Patron","This same little bird has also told me that my most recent \"business assiciate\" has been dealing with the head of the investigation.  Fortunately, this act of blatant disregard for the power structure in this corner of the galaxy has put no-one jeopardy, save my associate.  With the current confederate presence in this system however, any such action against said associate has been put on hold ... but it most certainly has not been suspended."),
                "With that, the patron closed his eyes and leaned back in his chair, and the men surrounding Deucalion walk back to the tables they rose from."]

gangsterhit1={"intro":[("Patron", "Deucalion! So good of you to come all the way out here to see me!"),
                    ("Deucalion", "Do I know you?"),
                    ("Patron", "No, certainly not. But you know a hauler named Jenek, who was unable to fulfill a contract with me."),
                    "This conversation was starting to sound like one Deucalion didn't want to be part of.",
                    ("Deucalion", "I'm sorry, I've really got to ..."),
                    "As he speaks, several people get up from nearby tables, and surround Deucalion",
                    ("Patron", "As I understand it, you were in a position to fulfill this contract, but did not.  This hurts me greatly, to see a man not willing to help another in distress. But don't worry, you have a chance to make it up to me.  Now it is up to you, do you wish to make up for this last mistake of yours?")],
            "reject1":[("Deucalion","I don't think I want to get involved in this."),
                    ("Patron", "I understand.  You are lucky I am a generous man, I will give you the chance to reconsider."),
                    "With that, the patron closed his eyes and leaned back in his chair, and the men surrounding Deucalion walk back to the tables they rose from."],
            "reconsider":[("Patron", "Welcome back, I knew you would see what must be done.  Are you ready to begin?")],
            "reject2":[("Deucalion", "No."),
                        ("Patron", "That is unfortunate. I would advise you to wait in this system until I have dealt with this matter."),
                        "The men surrounding Deucalion move just enough to allow him to exit.  As he leaves the table, every pair of eyes in the bar follow him. It was, to say the least, not Deucalion's finest hour."],
            "failure":[("Patron", "You never arrived with the package, and yet you do not have it with you.  You will not leave the system until I have decided what to do with you.")],
            "reminder":[("Patron", "My men are waiting. You should not keep them waiting much longer.")],
            "accept":[("Deucalion", "What does it involve?"),
                    ("Patron","Just a simple cargo run, a special package, to the hauler."),
                    "As much as Deucalion did not like the sound of this, he got the feeling he no longer had any choice in the matter.",
                    ("Patron","Just dock and have it unloaded, my men will take care of the rest.")]
        }

gangsterhit2=dict(gangsterhit1)
gangsterhit2["intro"]=[("Patron", "Deucalion!  So good of you to come all the way back out here to see me!"),
                    ("Deucalion","Luviccio?"),
                    ("Patron","Some call me that..."),
                    ("Deucalion", "Well, I got your message ... how do you know my name?"),
                    ("Patron", "Never mind that.  All that matters is that you are here. First of all, my thanks for completing that incompetent's cargo delivery. It is much appreciated."),
                    ("Deucalion","You're welcome."),
                    ("Patron","But now, I have another business proposition for you. You seem like a man who, like me, appreciates the value of a contract. Jenek broke his contract with me when he failed to make the delivery on time ..."),
                    "This conversation was starting to sound like one Deucalion didn't want to be part of.",
                    ("Deucalion", "I'm sorry, I've really got to ..."),
                    "As he speaks, several people get up from nearby tables, and surround Deucalion. The patron continues as if Deucalion hadn't spoken.",
                    ("Patron", "... and as a result some compensation is in order. I ask you, as a fellow businessman, if you would like to help me extract this compensation.")]

gangsterbounty={"intro":[("Patron","Well done Deucalion. You have won my confidence.  But I am just one man, and you do not get far in life with the confidence of just one."),
                        ("Deucalion","You have other plans for me then?"),
                        ("Patron","Plans?  Who is talking about plans? I was merely stating my satisfaction of a job well done."),
                        ("Deucalion","I--"),
                        ("Patron","But now that you mention it, I do have an opportunity for you.  This opportunity can provide you a way out of this backwater, and give you the lifestyle everyone dreams of."),
                        ("Deucalion","This opportunity, can it be refused?"),
                        ("Patron","Of course it can!  What kind of man do you take me for?  I would never force anyone into a job they didn't want.  However, I need an answer now ... I cannot tell you what this opportunity involves unless you can commit to it.")],
            "reject1":[("Deucalion","I'll have to think this one over.  From what you have told me, it is a big decision to make."),
                        ("Patron","That is okay, but do not take too long to make up your mind.")],
            "reconsider":[("Patron","You are back!  Have you made your decision yet?"),
                        ("Deucalion","Maybe."),
                        ("Patron","I'm afraid I won't be able to keep this offer open any longer.  If you are going to accept it, you must do it now.")],
            "reject2":[("Deucalion", "I'm sorry, I can't commit to such a large uncertainty."),
                        ("Patron","You dissapoint me Deucalion, but I will respect your decision.  Unfortunately for you, not all of my associates will.  I would be careful about leaving this system. Since the fighting began, the reign of law in this sector is not what it used to be.")],
            "failure":[("Patron", "You have cost me more by failing that mission than you ever could have by not accepting it."),
                        ("Deucalion","The target was expec--"),
                        ("Patron","Be quiet and listen.  There has been a Confederate investigation into activities of some criminals in this system.  Your mission was a personal favour to a friend of mine, as the target was a spy about to steal some very important information.  By failing in this mission, you have ensured the success of that investigation, and reduced any potential business in this system to none.  You can see how this makes my friend angry.  My friend no doubt will attempt to exact revenge.  Goodbye Deucalion.")],
            "reminder":[("Patron", "The target is on the move.  If you don't get him before he has left this system, you will have failed the mission.")],
            "accept":[("Deucalion", "I will take this opportunity."),
                        ("Patron","That is a good decision, one that I hope will be of benefit to both you and me."),
                        ("Deucalion","The job?"),
                        ("Patron","Yes, there is a ship that will soon be on a mission that will pose a threat to ..."),
                        "The patron turns away to answer a communication.",
                        ("Patron","The ship has just jumped into this system.  Instructions have been sent to your flightcomputer to target this ship's last known position.  You are to destroy it before it has a chance to jump to the next system.  Quickly, you must go now.")]
        }

gangsterreward=[("Patron","My associates, as well as I, wish to congratulate you on your success Deucalion.  In destroying that ship, you have ensured our organisation a few more years of uninterrupted operations in this sector.  As reward for this success, we have deposited a small amount into your account, and provide you access to all of our installations throughout the galaxy.  You have done well today."),
                "The patron stands up and shakes Deucalion's hand.  Only a few short days ago Deucalion was just another pilot making just enough to be comfortable.  Now he had the key to the galaxy!"]

hitsuccess="Hit on Atlantis:\\\\In yet another twist in the underworld wars raging through the Crucible sector, a little known cargo hauler, who cannot be named, is suspected dead after taking delivery of a shipment specified as \"Recycled Plastics.\"  With hangars CA-47 to CB-55 suffering irrepairable damage, investigators believe the entire shipment must have consisted of plastic explosives.  With such a scene an unfortunately common occurence in recent years, industry is quick at recovering.  Business is expected to return to normal at the spaceport Tuesday week, once remains of the 35 suspected dead have been found and identified.\\\\GNN -- Providing news for the galaxy."

confedwin="IntelSEC Investigation Completed:\\\\IntelSEC has today completed the preliminary investigation into a group of pirate organisations.  Operating out of Crucible Sector, in particular the system Cephid 17, this organisation has been found responsible for many offences, including many counts of murder, credit fraud, and several asassinations.  So far however, only the details relating to the counts of credit fraud have been made publically available.\\\\This is good news for the citizens of the surrounding systems.  The removal of the pirate menace has already seen local stockmarkets boom, and the Merchant's Union has already made plans to reopen disbanded trade routes.\\\\Aside from the criminals, the only others to lose out are several of the smaller financial institutions of the area.  Now compelled to reimburse the victims of the credit fraud, and with little hope of benefitting from the selloff of the pirates' assets, their share price has dropper by as much as 20% in the case of \'Crucible Holdings.\'\\\\GNN -- Local news, for a galactic audience."

piratewin="IntelSEC Investigation Cancelled:\\\\IntelSEC has today cancelled their investigation into alledged criminal organisations operating out of Crucible Sector.  Citing the murder of an investigating operative, as well as the general lack of anyone willing to provide testimonies, the agent leading the probe expressed his disappointment at the result.  \"Such an outcome will only serve to strengthen the grip of these organisations on the outer systems.  Every system with a pirate infestation is one that is lost to the Confederacy.\"\\\\The share markets in the involved systems took a battering today, with the only winners the smaller financial institutions.  Faced with claims of credit fraud, many have been quite lacklustre in their attempts to reimburse victims.  Some victims, 12 months later, still have their claims being assessed.\\\\GNN -- Local news, for a galactic audience."

def LoadMainCampaign():
    
    HAULER_SPRITE   = ("campaign/hauler.spr","Talk_To_The_Hauler") #sprite file for the fixer
    HAULER2_SPRITE  = ("campaign/hauler.spr","Talk_To_Jenek") #sprite file for the fixer
    HAULER_LOCATION = ("Crucible/Cephid_17","Atlantis")
    HaulerMission1  = CampaignClickNode() # Initialize each node
    HaulerMission2  = CampaignClickNode() # Initialize each node
    
    GANGSTER_SPRITE     = ("campaign/gangster.spr","Talk_To_The_Patron")
    GANGSTER_LOCATION   = ("Crucible/Cephid_17","Serenity")
    GangsterSwindled    = CampaignClickNode()
    
    Hauler2Consequence  = CampaignNode()
    HaulerReward        = CampaignClickNode()
    
    AngryHauler     = CampaignClickNode()
    AngryGangster   = CampaignClickNode()
    
    SPY_SPRITE      = ("campaign/investigator.spr","Talk_To_The_Investigator")
    SPY_LOCATION    = ("Crucible/Cephid_17","Ataraxia")
    INVESTIGATORID_SPRITE = "campaign/investigatorID.spr"
    INVESTIGATORID_POSITION= (0, -0.4)
    INVESTIGATORID_SIZE    = (1, 1)
    TailMission1    = CampaignClickNode()
    TailMission2    = CampaignClickNode()
    TailReward      = CampaignClickNode()
    TailRewardPop   = CampaignClickNode()
        
    GangsterSellout = CampaignClickNode()
    GangsterFight   = CampaignNode()
    
    GangsterHit1    = CampaignClickNode()
    GangsterHit2    = CampaignClickNode()
    GangsterBounty  = CampaignClickNode()
    
    GangsterReward  = CampaignClickNode()
    
    WrapUpSpy       = CampaignNode()
    WrapUpGangster  = CampaignNode()
        
    vs=Campaign("vega_strike_campaign") # Name of the save game variable for the entire campaign. Can't contain spaces
    vs.Init(HaulerMission1) # the first node.
    
    description = "Jenek:_Deliver_First_Cargo"
    MakeCargoMission(vs, # Creates a cargo mission
        HAULER_SPRITE, # Campaign, sprite
        [InSystemCondition(HAULER_LOCATION[0],HAULER_LOCATION[1])], # Where fixer meets you to start the mission
        [InSystemCondition("Redemption/Bernards_star","Jacobs")], # Where the mission ends. Usually the same as starting point for next fixer.
        None, # Script to be run as you click on the fixer. A common use is to AddCredits() for the previous mission.
        LoadMission(description,"directions_mission",(vs.name+"_mission",['Crucible/Stirling','Redemption/Bernards_star'], 'Jacobs')),#Script to be run to start the mission (usually None if you don't have a script, but ambush is also common.)
        ("Syringes",2,False), # Mission arguments.
        vs.name+"_mission", # Script to be set on completion. -1=Failure, 0=Not Accepted, 1=Succeed, 2=In progress
        haulerdeal1, # Dictionary containing what the fixer says.
        None, # If you reject the mission twice. "None" means that he continues asking you forever until you accept
        GangsterSwindled, # If you lose the mission
        HaulerMission2, # If you win the mission. Usually points to the next mission
        HaulerMission1) # The current mission node.
    
    description = "Jenek:_Deliver_Second_Cargo"
    MakeNoFailureCargoMission(vs, # Creates a cargo mission
        HAULER2_SPRITE, # Campaign, sprite
        [InSystemCondition(HAULER_LOCATION[0],HAULER_LOCATION[1])], # Where fixer meets you to start the mission
        [InSystemCondition(GANGSTER_LOCATION[0],GANGSTER_LOCATION[1])], # Where the mission ends. Usually the same as starting point for next fixer.
        ClearFactionRecord('merchant',0.7,PushRelation('merchant')), # Script to be run as you click on the fixer. A common use is to AddCredits() for the previous mission.
        LoadMission(description,"directions_mission",(vs.name+"_mission",[], GANGSTER_LOCATION[1])), # Script to be run to start the mission (usually None if you don't have a script, but ambush is also common.) (having no destination will call significant unit.. oakham should be the only dockable significant in that system
        ("Recycled_Plastics",50,False), #FIXME: varied cargo, and lots of it! Mission arguments.
        vs.name+"_mission", # Script to be set on completion. -1=Failure, 0=Not Accepted, 1=Succeed, 2=In progress
        haulerdeal2, # Dictionary containing what the fixer says.
        GangsterHit1, # If you reject the mission twice. "None" means that he continues asking you forever until you accept
        Hauler2Consequence, # If you lose the mission
        HaulerReward, # If you win the mission. Usually points to the next mission
        HaulerMission2) # The current mission node.
    
    HaulerReward.Init(vs,
        [InSystemCondition(HAULER_LOCATION[0],HAULER_LOCATION[1])],
        haulerreward,
        HAULER2_SPRITE,
        GoToSubnode(0,AddCredits(2000,ClearFactionRecord('merchant',1.0))),
        None,
        [GangsterHit2])
    
    GangsterSwindled.Init(vs,
        [InSystemCondition(GANGSTER_LOCATION[0],GANGSTER_LOCATION[1])],
        gangsterswindled,
        GANGSTER_SPRITE,
        GoToSubnode(0,SetCredits(0)),
        None,
        [TailMission1])
    
    Hauler2Consequence.Init(vs,[],None,None,TrueSubnode(),Hauler2Consequence,[AngryHauler,AngryGangster])
    
    AngryHauler.Init(vs,
        [InSystemCondition(HAULER_LOCATION[0],HAULER_LOCATION[1])],
        angryhauler,
        HAULER2_SPRITE,
        GoToSubnode(0,AdjustRelation('privateer','pirates',-0.05,AdjustRelation('privateer','merchant',-0.1,PopRelation('merchant')))),
        None,
        [GangsterFight])
    
    AngryGangster.Init(vs,
        [InSystemCondition(GANGSTER_LOCATION[0],GANGSTER_LOCATION[1])],
        angrygangster,
        GANGSTER_SPRITE,
        GoToSubnode(0,AdjustRelation('privateer','pirates',-0.05,AdjustRelation('privateer','merchant',-0.1,PopRelation('merchant')))),
        None,
        [GangsterFight])
    
    MakeMission(vs, # Creates any type of mission
        SPY_SPRITE, # Campaign, sprite
        [InSystemCondition(SPY_LOCATION[0],SPY_LOCATION[1])], # Where fixer meets you to start the mission
        [InSystemCondition(SPY_LOCATION[0],SPY_LOCATION[1])], # Where the mission ends.
        None, # Script on click
        AddRemovingSprite("investigatorID", INVESTIGATORID_SPRITE, INVESTIGATORID_POSITION, INVESTIGATORID_SIZE, "Give back the ID"), # Script to be run to start the mission (usually None if you don't have a script. Do NOT load an ambush mission here.)
        'tail',
        (vs.name+"_mission",0,["Crucible/Cephid_17","Crucible/17-ar","Redemption/Quetal"],30000,11000,"pirates","confed","Napolae","IntelSEC"), # Mission arguments.
        vs.name+"_mission", # Script to be set on completion. -1=Failure, 0=Not Accepted, 1=Succeed, 2=In progress
        confed_tail_finance, # Dictionary containing what the fixer says.
        GangsterSellout, # If you reject the mission twice. "None" means that he continues asking you forever until you accept
        GangsterFight, # If you lose the mission
        TailRewardPop, # Win mission
        TailMission1)
    
    MakeMission(vs, # Creates any type of mission
        SPY_SPRITE, # Campaign, sprite
        [InSystemCondition(SPY_LOCATION[0],SPY_LOCATION[1])], # Where fixer meets you to start the mission
        [InSystemCondition(SPY_LOCATION[0],SPY_LOCATION[1])], # Where the mission ends.
        PushNews(hitsuccess), # Script on click
        AddRemovingSprite("investigatorID", INVESTIGATORID_SPRITE, INVESTIGATORID_POSITION, INVESTIGATORID_SIZE, "Give back the ID"), # Script to be run to start the mission (usually None if you don't have a script. Do NOT load an ambush mission here.)
        'tail',
        (vs.name+"_mission",0,["Crucible/Cephid_17","Crucible/17-ar","Redemption/Quetal"],30000,11000,"pirates","confed","Napolae","IntelSEC"), # Mission arguments.
        vs.name+"_mission", # Script to be set on completion. -1=Failure, 0=Not Accepted, 1=Succeed, 2=In progress
        confed_tail_insider, # Dictionary containing what the fixer says.
        GangsterFight, # If you reject the mission twice. "None" means that he continues asking you forever until you accept
        GangsterFight, # If you lose the mission
        TailReward, # Win mission
        TailMission2)
    
    TailRewardPop.Init(vs,
        [InSystemCondition(SPY_LOCATION[0],SPY_LOCATION[1])],
        tailreward,
        SPY_SPRITE,
        GoToSubnode(0,AddCredits(50000,PopCredits(AdjustRelation('privateer','pirates',-0.55,AdjustRelation('privateer','confed',0.2))))),
        None,
        [WrapUpSpy])
    
    TailReward.Init(vs,
        [InSystemCondition(SPY_LOCATION[0],SPY_LOCATION[1])],
        tailreward,
        SPY_SPRITE,
        GoToSubnode(0,AddCredits(50000,AdjustRelation('privateer','pirates',-0.55,AdjustRelation('privateer','confed',0.2)))),
        None,
        [WrapUpSpy])
    
    GangsterSellout.Init(vs,
        [InSystemCondition(GANGSTER_LOCATION[0],GANGSTER_LOCATION[1])],
        gangstersellout,
        GANGSTER_SPRITE,
        GoToSubnode(0,AdjustRelation('privateer','pirates',-0.35)),
        None,
        [GangsterFight])
    
    description = "Luviccio:_Deliver_Cargo_To_Jenek"
    MakeCargoMission(vs, # Creates a cargo mission
        GANGSTER_SPRITE, # Campaign, sprite
        [InSystemCondition(GANGSTER_LOCATION[0],GANGSTER_LOCATION[1])], # Where fixer meets you to start the mission
        [InSystemCondition(HAULER_LOCATION[0],HAULER_LOCATION[1])], # Where the mission ends. Usually the same as starting point for next fixer.
        None, # Script to be run as you click on the fixer. A common use is to AddCredits() for the previous mission.
        LoadMission(description,"directions_mission",(vs.name+"_mission",[], HAULER_LOCATION[1])),# Script to be run to start the mission (usually None if you don't have a script, but ambush is also common.)
        ("Low_Yield_Explosives",1), # Mission arguments.
        vs.name+"_mission", # Script to be set on completion. -1=Failure, 0=Not Accepted, 1=Succeed, 2=In progress
        gangsterhit1, # Dictionary containing what the fixer says.
        TailMission2, # If you reject the mission twice. "None" means that he continues asking you forever until you accept
        GangsterFight, # If you lose the mission
        GangsterBounty, # If you win the mission. Usually points to the next mission
        GangsterHit1) # The current mission node.
    
    description = "Luviccio:_Deliver_Cargo_To_Jenek"
    MakeCargoMission(vs, # Creates a cargo mission
        GANGSTER_SPRITE, # Campaign, sprite
        [InSystemCondition(GANGSTER_LOCATION[0],GANGSTER_LOCATION[1])], # Where fixer meets you to start the mission
        [InSystemCondition(HAULER_LOCATION[0],HAULER_LOCATION[1])], # Where the mission ends. Usually the same as starting point for next fixer.
        None, # Script to be run as you click on the fixer. A common use is to AddCredits() for the previous mission.
        LoadMission(description,"directions_mission",(vs.name+"_mission",[], HAULER_LOCATION[1])),# Script to be run to start the mission (usually None if you don't have a script, but ambush is also common.)
        ("Low_Yield_Explosives",1), # Mission arguments.
        vs.name+"_mission", # Script to be set on completion. -1=Failure, 0=Not Accepted, 1=Succeed, 2=In progress
        gangsterhit2, # Dictionary containing what the fixer says.
        TailMission2, # If you reject the mission twice. "None" means that he continues asking you forever until you accept
        GangsterFight, # If you lose the mission
        GangsterBounty, # If you win the mission. Usually points to the next mission
        GangsterHit2) # The current mission node.
    
    MakeMission(vs, # Creates any type of mission
        GANGSTER_SPRITE, # Campaign, sprite
        [InSystemCondition(GANGSTER_LOCATION[0],GANGSTER_LOCATION[1])], # Where fixer meets you to start the mission
        [], # Where the mission ends. Usually the same as starting point for next fixer.
        AdjustRelation('privateer','pirates',0.2,AdjustRelation('privateer','merchant',-0.1,PushNews(hitsuccess))),
        None, # Script to be run to start the mission (usually None if you don't have a script. Do NOT load an ambush mission here.)
        'bounty',(0,0,0,False,0,'confed',(),vs.name+"_mission",'','',True,[]), #Mission arguments.
        vs.name+"_mission", # Script to be set on completion. -1=Failure, 0=Not Accepted, 1=Succeed, 2=In progress
        gangsterbounty, # Dictionary containing what the fixer says.
        GangsterFight, # If you reject the mission twice. "None" means that he continues asking you forever until you accept
        GangsterFight, # If you lose the mission
        GangsterReward,
        GangsterBounty)
    
    GangsterReward.Init(vs,
        [InSystemCondition(GANGSTER_LOCATION[0],GANGSTER_LOCATION[1])],
        gangsterreward,
        GANGSTER_SPRITE,
        GoToSubnode(0,AddCredits(10000,AdjustRelation('privateer','pirates',0.2,AdjustRelation('privateer','confed',-0.5)))),
        None,
        [WrapUpGangster])
    
    GangsterFight.Init(vs,
        [],
        [],
        None,
        GoToSubnode(0,(LoadMission("ambush","ambush",(vs.name+"_mission",universe.getAdjacentSystemList(GANGSTER_LOCATION[0]),0,'pirates',4,'','',["You shouldn't have left that system, Luviccio hadn't finished with you.", "No matter, we'll finish you for him!"])))),
        None,
        [WrapUpSpy])
    
    WrapUpSpy.Init(vs,
        [],
        [],
        None,
        GoToSubnode(0,(PushNews(confedwin))),
        None,
        [CampaignEndNode(vs)])
    
    WrapUpGangster.Init(vs, # savegame variable
        [], #start location
        [], #dialog
        None, # fixer sprite
        GoToSubnode(0,(PushNews(piratewin))), # subnode script
        None, # completion script
        #campaign_pirates.Mission1) #[CampaignEndNode(vs)]) # continue with next mission
        [CampaignEndNode(vs)]) # continue with next mission
    
    return vs
