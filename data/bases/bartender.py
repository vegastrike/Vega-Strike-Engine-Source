import Base
import VS
import vsrandom
import fixers
tender={"general":("Hey, pal. What'll it be?",
                   "Bottoms up for ye olde bartender, eh?",
                   "Say, don't I know you from somewhere...?",
                   "Got any good stories to tell? I'm out, and I need some fresh ones to tell my regulars.",
                   "Tilt back a nice cool one, friend.",
                   "News is sparse around here, check back with me later when I've talked with some more of the haulers coming in later today.",
                   "Shh! Looks like someone is watching. Keep your six covered.",
                   "Have one on the house, it's happy hour!",
                   "A nice drink and a bit o' talk is what I live for.",
                   "Have you heard what's been on the news recently? It ain't pretty!",
                   "Can I get you anything? Whiskey? Vodka? Aarnbach's Acid? I'm all for being social, but I do have a business to run.",
                   "Did you know that before I started working here, everyone was served by either a robot, or computer controlled cyborg? I mean, I'm happy to work here... but who could stand having an unfeeling machine as a bartender?",
                   "I've run out of stories for the local crowd. Why don't you tell me a few tall tales from your adventures that I can make a few feet taller for the regulars?",
                   "You're in luck, this one's free! You just caught the tail end of happy hour!",
                   "Here, have a drink. This is a classic, it's called 'Venutian Sunset'.",
                   "I just got a shipment of fine scotch in from good ol' Earth. Care for a taste?",
                   "Sorry pal, can't talk now.",
                   "Been paying attention to any of the interplanetary sports? Yeah... me neither.",
                   "Keep your hand close to that gun of yours... I think one of those shady-types has their eye on you.",
                   "Hello! Lots of travellers about. Have yourself a seat and enjoy a nice cool fuzzy buzzer.",
                   "Does he pat YOU like that?",
                   "I think the Luddites are crazy, just like anyone else. But I have to admit, their lobbyist friends in the Purists got the Senate to pass a law BANNING non-sentient robots and droids from serving alcohol. And that friend, is good news for me!"
                   "Can I get you anything? Whiskey? Vodka? Aarnbach's Acid? I'm all for being social, but I do have a business to run.",
                   "I don't have anything good for you right now. Check back in later after I've talked to some other folks.",
                   "We'd better not talk right now. I think you're being watched.",
                   "Try this, it's my latest creation. A Jovian Sunspot!",
                   "If you're as much a smuggler as I think you are, you had better watch your P's and Q's around this place.",),

        "university":("You'd best keep your sidearm covered in this establishment, sir. Students don't take kindly to threats!",
        "One of the great things about running a bar on a university planet...  is that there is quite an 'archive' of alcoholic beverages stored for both 'historical' purposes and 'hands-on' learning.",
        "I went to a university planet myself back in the day. Not this one, of cousre, but a nicer one. It was really great to live on a planet full of 'loose' teenagers and open liquor cabinets. Now I'm married though...  and I had to swear off alcohol.",
        "There was an ISO discussion group in here last night. I let them use the bar, because they're good customers.  Some of them don't believe in money, though, so it can be kind of tense trying to collect on their tabs sometimes.  Mostly, I'm afraid if I push too hard, I'll come to work one afternoon and find my bar burned to the ground!  Crazy revolutionaries.",
        "You might think it's boring on a university planet. But let me tell you... there's no other place to be.  There're all kinds of fancy cultural artsy things going on, sporting events, music concerts, political groups, revolutionaries...  it's the place to be if you want to be on the cutting edge of society.",
        "Eighty percent of the population of a university planet is between the ages of 18 and 24.  So that means forty percent of the population is a young person of the opposite gender. University planets are like the ultimate place for singles, and I run a bar there. So it's like I'm running the ultimate singles bar.",
        "Sometimes I really hate living on a university.  There are so many snobs, arrogant rebels, naive revolutionaries, violent protesters, anarchists, deadbrains, drunkards, and rich idiots.  And they ALL come through MY BAR!",
        "There was a performance by the local symphony orchestra last night. I couldn't go, 'cause of the bar, but I hear they played some great classical composers. Some Beethoven, some Beatles, and some Suguro. Great stuff!",
        "There's some really incredible research going on at the Advanced Spatial Theory Laboratory here.  They say they're close to developing an interstellar jump drive that doesn't need to utilize pre-existing wormholes!  It's incredible research because it's a wonder it's getting any funding.  They've been working on that for almost a century, and no progress!",
        "There are some very interesting studies going on at the campus here.  On grad student was telling me about how he was working on 'The Mating Habits of the Rlaan Component Species'. If you ask me, that will make for some pretty good reading.",
        "We had some Aera corpses delivered here recently. The Confederation Navy scooped them up after a battle, and asked us to do some classified research on them. They were only sent here AFTER the Navy intel boys got through with them of course.  I wonder what they figured out, and if they're telling us everything.",
        "The big money these days is in heavy weapons developement and dentistry.  So when the kids with credit signs in their eyes figure out that they either CAN'T do the work, or HATE it, they switch their majors to English or history, or orbital engineering.  You know, stuff that's easy.",
        "Contracts for defense research have skyrocketed since the Aera turned out to be hostile.  It's easy money if you're into starship or weapons design.  It drives the pacifist student groups wild, but the military has doubled the size of their officer training programs on university planets too.  Interesting times to be on a university planet.  Or anywhere, I guess.",
        "There is entirely too much talking going on in this bar...  and not enough drinking!  DAMN intellectuals.",
        "To the university!  Home of lost causes, and forsaken beliefs, and unpopular names, and impossible loyalties!",
        "You know, university degrees are a bit like adultery: you may not want to get involved with that sort of thing, but you don't want to be thought incapable.",
        "The Confederation authorities busted a local discussion group last month, and the fallout has been scathing.  Who thought that anyone would actually enforce that silly law they passed banning the Interstellar Socialist Organization?  I mean, they're radical and all...  but it was just a discussion group!",
        "There was a pro-ISO rally in the central square of the local campus yesterday.  They started calling for all sorts of government and economic reforms, but some hot-heads eventually turned to advocating bloody revolution instead.",
        "A wing of fighters flew into port last week.  No-one I know saw who the pilots were, but everyone suspects that they were part of an ISO flightgroup.  My guess is that they were lying low for a while, waiting for some Confed heat to cool.  Small wonder they chose here, they've got all kinds of supporters on this kind of planet.",),

        "advice":("If youre looking for a mission, talk to the guys who sometimes sit at the front tables there. Or you can check the mission terminal.",
        	"I used to know a privateer who swore by a standard mining base-agricultural planet run.  He hauled grain one way, and raw metals the other.",
        	"You wouldn't catch me in space without a good shield generator. I want to be able to have time to decide if I want to run or fight.",
        	"Hey, why don't you see if there are any in-system bounty missions available? People around here would really appreciate any help in cleaning this system up.",
        	"The jump drive is the key to the galaxy. Without one, you can't access the network of jump points we use for interstellar travel.",
        	"The best tool for interplanetary trade is common sense. What does an agricultural colony have that a space station probably needs? That's right, food - and booze!.",
        	"You have to open your communications menu and request docking clearence before a planet or station will let you land.",
        	"There is a commodity exchange at every civilized (and most uncivilized) port in the known galaxy. Why? Because trading is a great way to make honest money.",
        	"When dogfighting, you can match speeds with your target (HOME key).",
        	"You can cycle through the screens on your HUD (V key), and communicate with your targets.  You can taunt them or hold a conversation!",
        	"If you get in a fight, it is very important that you lock onto and concentrate on a single target (L key). This will make you targeting box a solid square, and your computer won't lose your target in the heat of battle.",
        	"It's a dangerous galaxy out there. A lot of new pilots have accidently wandered into Aera space before they were ready, and ended up floating home.",
        	"The first thing a pilot needs to know to be successful, is how to fly his ship. If you aren't an old hand at the flight stick, I would suggest printing off a list of controls to glance at while you're off making your fortune hunting pirates.",
        	"Space is big. A pilot has to utilize his ship's in-system jump engine to get from planet to planet in less than a month (A key to travel to target).",
        	"Make sure you bring a few tracking missiles with you, like FFs or IRs. They can help you take down an enemy more quickly than just using your guns.",
        	"Watch your primary capacitors as you fire your guns. If you're running out of energy too quickly, considering buying a more powerful reactor to produce more power.",
        	"Don't attack ships from groups that you want to be friends with. If you make the Merchants angry enough, they'll put a bounty on your head so big that every bounty hunter in sector would be after you!",),

        "tale":("Got any stories to tell? Last week I had a patron who just could not stop talking about his battles with pirates.",
                    "This fellow brought a jhorg in here yesterday. Ever heard about the jhorg? They're small scaly creatures from New Bremen that can mimic everything you say with perfect accuracy. Forget parrots, these things are better than tape recorders!",
                    "So, clue me in on something - why do Andolians still sometimes go to bars if all they're going to order is water, and they won't even drink that until they run it through the purifiers in their suits? I mean, look around, it can't be for the ambiance. I tell you, sometimes it scares me trying to figure out what they're thinking inside those suits. Sitting in the back of my bar, staring at the wall and seeing something only the other linked can see - if I still had skin it'd give me goosebumps.",
                    "A couple of goons from the IntelSec came in here earlier looking for some guy named Jensen. Wouldn't say what it was about, but let me tell you, whatever it was, I was happy when they left. I swear, half the bar found excuses to be elsewhere the moment those two guys entered, and I don't blame them.",
                    "Shaper woman comes in here a few weeks back, wearing living clothes, naturally. Some drunken Purist, maybe he was a Luddite sympathizer or just really stupid, he comes up and starts yelling about how she's a desecration of the human species or some junk - next thing you know, some vine from her dress has the guy face down on the bar in a chokehold. Scary thing is, she didn't even bat an eye. She just was smelling the wine I'd just served her the whole time as if nothing was happening. When his unconscious body slid off my bar, I tell you the smile she gave me could have frozen helium.",
                    "A woman came in here the other day carrying one of those fancy Aera energy pistols in her belt. Don't ask me how she managed to get her hands on one of those, but even the roughest of my customers made sure to stay out of her way.",
                    "This captain of a Wayfarer came in here the other day, looking like she could really use a drink. I'd figured anyone flying a ship that many decades old might need a couple just to calm their nerves, but it turned out she had been chased by pirates across three systems! Persistant little buggers, aren't they? And the Regional Guard wasn't much help for some reason.",
                    "A mercenary outfit, the 'Red Condors', came in here a while ago. Real tough-looking types. I expected trouble, but they quietly finished their drinks and left. Glad there's still some professionalism out there.",
                    "There was this one merc in here the other day... he told me that he and the Aera he was fighting crashed on some world on the frontier. There was no human-edible food, so he said he lived off the dead Aera for three weeks! Can you believe that?",
                   "This Highborn youth and his lackeys come in last month - boy is total glutton, worst shape I've seen an aristo in in years. Apparently he's got speciesist tendencies among his failings too, because first thing he and his friends do is try to pick a fight with one of the Klk'k at the bar. Rest of the human bar patrons suddenly jump up from their seats in unison, completely clockwork-like - turns out they were all Andolian-steppers who were taking their Bro-chan out for a promotion party. As soon as that Highborn kid noticed the links on the backs of their necks... man, I don't know that I've ever seen a guy that out of shape run that fast.",
                    "Some privateer was in here a while back, bragging about his adventures in unknown space. He kept talking about these wonderful alien artifacts that he used to improve his ship. Claimed they were Betan... or was it Alphan? I can't remember, and I bet you 10 to 1 that chump couldn't have told the difference!",
                    "Every so often, an Uln comes in here. They have their own civilian sector pretty much like our own, and sometimes a couple of their privateers find their way across the border. But let me tell you, never give them beer. One of them had that, and he got into some sort of violent spasms and threw up over the whole bar. Not a pretty sight.",
                    "Hello there, friend. What can I get you? Sorry, I don't have any of the good stuff left. I had a transport bringing in new beverages from the Uln border, but there was an... accident. Apparently, some of that stuff is more flamable than napalm! How was I to know?",
                    "I think that the craziest story I've ever heard in this bar...  is where this pilot says he was out on the Aeran frontier, flying alone in an uncolonized system. He's running pretty much with zero emissions, so no Aera patrols can spot him. He's thirty thousand clicks out from the jump point, when his passive sensors go CRAZY. He says thats when a sparkling, glowing green orb popped out of the jump point, and just took off to another jump point at insane speeds. The pilot sold his ship right after he left my bar.",
                    "...and so there I was. Regional Guard patrol to my port, Confed fighter wing to my starboard... and my dumped cargo coasting slowly away from me about 600 kliks to stern. I thought I was home free. Suddenly, they all started powering up their weapons. 'I'm fracked', I thought. Turns out there was a wing of Aeran Nicanders on an attack vector. Then I thought 'Now I'm REALLY fracked'.",
                    "There was a guy in here the other day who claimed he was the sole survivor of some colony massacre by the Aera out on the frontier. No one believed him for a minute though - if the Rlaan ever caught the Aera slaughtering a colony's worth of civilians, the Rlaan would be massing on the Aeran border in a heartbeat.",
                    "My brother signed up for the New Farmers of the Confederation program, and got shipped out to a new agricultural colony out on the frontier. I got a trans from him last week, and he said that life was really hard compared to Mars, but that he was happy to be living on a surface habitable planet finally.",
                    "By Jupiter, there are some crazy people out there.  Just last week, a shipment of industrial goods destined for the base here got shot up by a wing of Luddite fighters. It's insane!  They lash out against modern technology, but they use the latest weapon systems and starfighters to do it!",
                    "There's got to be someone behind the Luddites. I mean, fanatical social reactionaries don't have the money or the resources to buy ships, train pilots, or construct highly organized terrorist operations all by themselves, even with a bunch of whacked out, tithing followers.  Now who would stand to gain from the Luddite craziness?  Hmmm....",),

        "confednews":("Have you heard the news? Buster Friendly just died. Remember his show? Buster Friendly and his Friendly Friends? I grew up on that show. I think I need a drink.",
        	"The criminal consortiums out on the frontier are getting more organized by the day. I hear they're extorting entire colonies and systems. Some of the guys are placing bets on what will bring about our fall first. The pirates from within or the Aera from the outside?",
                "There's a lot of people complaining about the pirates, but I don't mind them so much.  The Confed and the Merchies know they need a running merchant operation in the outlying systems, but they haven't got the resources to patrol every trade lane for pirates, so they need people who can look after themselves. People like you and me. And that's why they go easy on drafting our kind.",),

        "hook":("Myself, I used to fly for the Exploratory Service til I saved up enough to buy this bar. Its a good living. Theyre not real picky about who they take and the pays nothing to sneeze at. If you dont mind getting shot at, from time to time, you oughtta look into it.",
        	"I've heard some funny stories of strange things going on in the Enigma system. It supposedly has something to do with the black hole there, but that's only the official explanation.",
        	),

        "aerawar":("Hey, did you hear the news? Word is that the 5th Fleet under Admiral Johansson managed to bag a whole bunch of those nasty Aeran cruisers in a battle not far from here! Maybe this war is winnable after all!",
        "A toast to the men and women of the Confederate Armed Forces! If not for them, we'd all be learning how to speak Aera right about now. If those things even have vocal chords.",
        "We might have had some luck raiding some of the outlying Aeran outposts, but I hear their planets are going to be a tough nut to crack. An old friend of mine from the HCS Zhukov told me that, back during the Rlaan-Aera war, they tracked a Rlaan battle fleet as it entered orbit around one of the planets, but then it simply disappeared off their scopes! The fleet that is, not the planet.",
        "So what's your take on the war? A friend of mine in the Navy told me that the Aera-Rlaan border is still deadly silent. Word is that the unofficial cease-fire might as well be an official one. Some are even saying the Aera might redeploy even more of their fleet units from that front. If that happens, the Aera will bring all of their high-tech weapons to bear on us, and I don't just mean those poor bastard in Forsaken space. Things aren't looking good.",
        "It's hard to tell what this war with the Aera is going to be like, because we aren't really into it yet. I suggest you don't put too much weight on the first few skirmishes... I suspect we'll be in this one for the long haul.",
        "Have you heard that the 5th Fleet and the 9th Fleet are actually made up of mostly the same ships?  It's the 5th Fleet when under Admiral Johansson, and the 9th when under Admiral Nogura. The 3rd and the 4th Fleets are the same way. I hear it confused the hell out of the Aera for a while. They thought we were magically growing new fleets out of our armpits!",
        "I've got a sister in the Navy. She's a weapons officer in the 6th Fleet out near the Aera-Confed-Shmrn border. I asked her to fry a few aliens for me, but so far neither side is committing in full to that sector. To be honest, I still hope she doesn't have to actually fight against a full on Aeran assault - it's murder out there, but I wanted her to think I wasn't worried.",
        "I've got a buddy in the Confed Marines, attached to the 10th Fleet. He's out on the Rlaan border, corward of Shaper space, the complete opposite side of the Confederation from the fighting. He complains about it being boring out there... but if I was him I'd be pretty happy.",
        "There's a rumor going around that the Aera have some big capital ships deep inside their home territory. I don't mean just big... I mean BIG big. Big enough to use one of the old Starrunners for a pinnace. Then again, it's just a rumor.",
        "I've heard that the Aera don't have a very well-defined concept of 'prisoners of war'. The Rlaan help keep them honest when it comes to civilians, but I hear that the Navy started to give cyanide pills to pilots in case the wrong side picks them up after they bail out.",
        "The Aera might seem to be more powerful than us at first glance... but how much of their economy is dedicated to war? Most of it, I'd say. The Confederation spends only a few percent of its gross product on the military. If push comes to shove... we'll knock 'em over for sure, provided we can ever get the Senate to agree to anything useful before the whole bloody war is over!",
        "I've heard that the Confed Navy has been trying to slip intel teams behind and around Aeran lines to set up listening posts. I'd hate to be working that duty...  out there, all alone. Only kilometers away from Aeran fleets. Always afraid of being discovered...  brrrrr.",
        "You know what they say... 'War is good for business'?  Well it ain't true. How many pilots that were regulars here have been drafted?  How many freelancers have gotten fried on escort duty?  A lot.  That's a lot of people NOT buying drinks from me.  Lousy war.",
        "Have you heard about the anti-war protests back home?  I could understand that if we were invading Aera space, being aggressive and all...  but those protesters have NO idea what's going on out on the Frontier, do they?  Not one damn clue!  Maybe someone should send them out to the Justman Colony, so they can see just how 'peaceful' the Aera really are!",),

        "wisdom":("Man, you go through life, you try to be nice to people, you struggle to resist the urge to punch 'em in the face, and for what?",
        "Listen, here's the thing. If you can't spot the sucker in your first half hour at the table, then you are the sucker.",
        "Remember, ability can take you to the top, but it takes character to keep you there.",
        "Eat, drink, and be merry... for tomorrow we die!",
        "When sorrows come, they come not single spies, but in batallions.",
        "Be not afraid of greatness. Some are born great, some achieve greatness, and some have greatness thrust upon 'em.",
        "Cowards die many times before their deaths...    The valiant never taste of death but once.",
        "I will eat with you, drink with you, talk with you, and so following; but I will not trade with you, fly with you, nor pray with you.",
        "All that lives must die... passing through nature to eternity.",
        "Women will forgive anything. Otherwise, the race would have died out long ago.",
        "Happiness is having a large, loving, caring, close-knit family... on another planet.",
        "If you're going through hell, just keep going.",
        "Beware of small expenses; a small leak will decompress even a great starship.",
        "Our doubts are traitors, and make us lose the good we oft might win by fearing to attempt.",
        "This above all: to thine own self be true! And it must follow as the night the day, thou canst not then be false to any man.",
        "There are two kinds of adventurers: those who go truly hoping to find adventure, and those who go secretly hoping they won't.",
        "Without adventure civilization is in full decay.",
        "Ask five economists and you'll get five different explanations. Seven if one went to Berkeley in the Defiance system.",
        "No one is rich whose expenditures exceed his means, and no one is poor whose incomings exceed his outgoings.",
        "I HAVE TEH POWER",
        "Prometheus is reaching out for the stars with an empty grin on his face.",
        "The eternal silence of these infinite spaces fills me with dread.",
        "Where does one not find that bland degeneration which beer produces in the spirit!",
        "The worst thing about some men is that when they are not drunk they are sober.",
        "Some men are like musical glasses; to produce their finest tones you must keep them wet.",
        "It was a woman who drove me to drink, and I never had the courtesy to thank her for it.",
        "The cost of living has gone up another credit a liter.",
        "Glory is fleeting, but obscurity is forever.",
        "To refuse awards is another way of accepting them with more noise than is normal.",
        "The first casualty when war comes is truth.",
        "The military don't start wars. Politicians start wars.",
        ),



         "military":("A Special Forces unit came through here this morning. Real quiet guys. Looked like they'd seen a lot of action. From what I could gather, they were going to catch a transport out of here and into the fray again later tonight.",
                    "This Army captain had a drink here yesterday. Tall guy from Mars. Now, I've seen a lot, but this guy... half his face, both of his legs and his left arm had all been replaced with cybernetics due to injuries he'd taken on the field. If it was all with the Mechanist aesthetic, it might have been impressive, but he visually reeked of military hack job. He looked a bit clumsy, it was obvious there hadn't been many nerve endings left to connect to anything.",
                    "Some idiots from the ISO came in here yesterday handing out phamphlets and bothering the customers. They wouldn't take a hint, so I had them thrown out. Some days I almost don't know who's worst, the ISO or those Luddite religious fanatics that fight with you all the time.",
                    "You know about those Rlaan? Supposedly, they never attack unarmed vessels. Well, word is there's this rogue Rlaan corvette hiding out near here. Fled to our side of the border when their own Enforcers caught up with it. I hear the whole crew went completely sideways in the head after some botched gray-op, and now it'll attack anything or anyone that comes in its way.",
                    "A couple of guys from the Confederate Air Corp were here a while back. We don't get many of those here usually. They may rely on the Navy for transportation between planets, but those Air Corp boys and girls are solid gold when it comes to campaigns planetside!",
                    "Hey, are you a mercenary? We've got plenty of missions here for you to fly, if you're willing. The fleet is spread thin as it is, so if you want to do your part, you're more than welcome!",
                    "You like you're a bit rough around the edges, friend. I'd watch myself around here... hardly anyone around but military types who probably don't take too kindly to people like you crowding them."
                    "I suggest you keep that sidearm well holstered, pal. For your own safety.",
                    "One of the most interesting things to check out if you're in a military bar that pilots frequent is the local kill board. You can pretty quickly figure out who's who that way.",
                    "It's all clear now, but if you ever walk into a bar where the pilots are all sitting on one side, and the ground pounders on another... always keep a quick path to the exit handy.",
                    "Military bars are a good place to pick up good stories. Always a pilot telling about some crazy encounter his wing had with the Aera, or some Marine talking about how he got in a bar fight with an Uln or something.",
                    "Don't the new fighter models look pretty dang sleek this year? I wish I was still flyin'. Well, with the war and all... I ALMOST wish I was still flyin'.",
                    "There was a big meeting between Navy brass and Navy Intelligence last week. Two of the analysts from the Aera intel division came in afterwards. Boy, did they drink a lot.",
                    "Whatever you say about cocky, hotshot Navy pilots... you have to admit that they sure can fly their ships.",
                    "Military installations and ships are good places to pick up some premium surplus milspec merchandise. You can make quite the pretty penny running guns in these tumultuous times.",
                    "Hey, I got something special for you. I gave a pilot some advice that ended up saving his life, so he bought me a bottle of top quality Crystalia Ale. So, now I'm sharing the wealth! Drink up!",
                    "Do you know the motto of the Confed Navy's Scouting and Deep Listening Post program? 'The price of freedom is eternal vigilance'. Sounds like its from a movie or something.",
                    "Hey, you look like a bit of a privateer, maybe you can use this tip. Some of my buddies in Navy Intel have been telling me that they desperately need Aera weapon systems to analyze. I'm willing to bet that there would be some GOOD money in it for anyone who ran some Aera guns over to the Confed side and dropped them off at an R&D facility - provided they were still operational.",
                    "You can bet that as the war heats up, the Navy will start to get spread pretty thin in the interior systems. That will mean lots of opportunities for privateers. Opportunities to break the law, or to enforce it.",),

	"mining":("You want to know what the big story around here is? Industrial gems. They're just about the most valuable raw material known to man, and we just stumbled onto a rich vein of them. That ought to keep this base, and this bar, in the black for decades to come!",
		                      "Confed has been treating us miners pretty well lately. In terms of giving us adequate defense coverage, I mean. They're still trying to give us the shaft when it comes to mandatory ore contracts. I mean, there's a war on and all... but the boys who come in here need to be able to pay for their drinks!",
		                      "I was a prospector back in the day... I scouted out new asteroids for mining operations. I got the standard .05% commission on the first year of net profits on two different asteroids. I was rolling in the dough for weeks, until I lost it all in a bad investment. Kept enough for this bar, though.",
		                      "I've been hearing some grim reports from the front lines. I hope we're far enough away from all that, because I'd sure hate to play host to a squadron of Aera raiders if our militia protection was off on some mission.",
		                      "You won't need that gun in here, pal. Nobody around but us miners.",
		                      "You probably can't tell, but base humidity is up a bit this week. I don't know if it's because they're trying to give us some weather, or because the climate controls are just broke!",
		                      "We had a bomb go off in a newly arrived shipment of cybernetics last month. Nobody got killed, but a few good miners had to get replacement limbs. We got a message from the Luddites claiming responsibility. Talk about your irony! Those psychos are completely insane.",
		                      "Here's a funny one: Why did the mine shaft collapse? ... Because the droids were boring it too much! Harharharharhar!",
		                      "I heard from a buddy on a refinery station ilke four jumps from here that refineries in Confed space are desperate for our raw ore! Iron, tungsten, radioactives... there's a lot of demand for starships right now, and lots of refineries don't have enough raw material to meet demand.",
		                      "Do you want to know something strange? You can almost always sell the industrial gems mined here on agricultural worlds, even though they have absolutely no use there. I figure the people who run the local exchanges plan on selling them for an even higher price once they go back to  civilization, or something. Who knows!",
		                      "Hey, I'd love to swap stories with you, but a shift just came out of the tunnels, and it's about to get REAL busy in here.",
                                      "Are you going to stick around for the big festival? Every year, if profits are good, the base governor throws a big party. Quite a spectacle. Fireworks, plenty of luxury food, the whole nine yards. If the Luddites or ISO dont assassinate anyone, it ought to be a real good time.",
                                      "It can get pretty stale, living out here on a rock like this, with only a few thousand people to keep you company. I guess that's why mining bases are such large consumers of entertainment...  and drugs.",
                                      "The base constable was in here the other night, breaking up a fight between some drunk miners who were having a 'disagreement' about how big a stake each one had in the recent uranium vein discovery.",),



	"agricultural":("Have you heard the news?  The Lars family brought a melon to market yesterday that was four meters 'round!",
	                        "This year's crop is going to be huge!  I can't wait until the harvest festival, I'll sell so much beer! You try and make it, if you're in the system!",
	                        "Some farmers fifty or sixty miles from the colony center or so have been talking about seeing some real strange things lately.",
	                        "We're just simple folk here. Keep your nose out of trouble and no trouble will come to you, I say!",
	                        "A hotshot pilot in here the other day said that Farmer Haggot has some of the best pipe weed within two jumps of here!",
	                        "Did you hear about how the Miller boys all joined the Confed Navy together? I don't know what their father is going to do when he's too old to run the farm...",
	                        "Did you know that our colony was selected by the Confed Agriculture Authority to test some new engineered crops? They aren't on the market yet, but by being the first to test them, we'll get exclusive rights for six years!",
	                        "Here's a good one: What do you call a potato that can't keep his lunch down? ... ... a regurgiTATER! Harharharhar!",
	                        "People around here don't like to talk about it, but we're all afraid that we'd be an easy target for an Aera attack. We don't have any orbital defenses, no fighter wing... heck, we don't even have a ground battery. All it would take is one Aera bomber with a fusion warhead to take out half the colony! I don't care if everybody says it won't happen like that because the Rlaan would raise hell afterwards, because by that time we'll already be dead!",
	                        "My grandpa grew up in Iowa on Earth before he moved out to this colony world, here. Wanted to make a new life, and raise corn the way HIS grandpa did.",
	                        "I visited one of those industrial worlds a few years ago... So many people, and you couldn't even see the stars at night! Still, not as scary as an Andolian world. I visited one of those once - and once only. Normal people can't live like that, not even seeing the sky, let alone the stars",
	                        "I just heard this one the other day: What do you call a potato that can never make a quick decision? ... ... a hesiTATER! Hahahahahaha!",
	                        "I just thought this one up: What do you call a potato that likes to watch sporting events? ... ... a specTATER! Hehehehehehehe!",
	                        "What do you think of this one: What do you call a potato that likes to talk about football games on the colony radio? ... ... a commenTATER! bwaHAHAHAHA!",
	                        "Did I ever tell you the one about the farmer who was outstanding in his field?",
	                        "I just heard about a wheat blight that struck another agricultural colony a few jumps away. They say that it might not have been a natural outbreak...",
	                        "A lot of those crazy Luddites come from agricultural worlds that were founded years ago as isolated subsistence colonies, and have only recently had their economy retooled for mass export. Some people just can't take the shock of all the heavy machinery and laborer robots that take over practically overnight. I respect wanting to live in the fashion of your ancestors as much as the next fellow, but trying to reverse half the societal changes and opportunities of the last millenia is just crazy talk.",
	                        "You know why the cow jumped over the moon? ... ... the farmer had cold hands! Harharharharhar!",
	                        "What do you get what you cross a cow with a rooster? ... ... cock-a-doodle-moo! Heh, okay that was bad.",
	                        "Friend, you look like a hard man. The kind who looks for dirty work. But this is just an agricultural base. Were farmers, not pirates looking to hire cut-throats and thugs. Please, dont make any trouble here.",
	                        "The Duntley boy said he saw some misshapen figures lurking behind the barn late one night last week. He said that they looked like Aera in stealth suits. We all told Duntley he was a fool for letting his son have a computer system..."
	                        "Hey, stranger! We were just having a debate here, and maybe you could help us settle it.  What do they prefer where you come from...  okra or zuchinni?",),


	"pirates":("You've got a lot of nerve coming in here looking like that, pal! This joint enforces a strict dress-code. Here, put this eye-patch on before you cause a scene. And ruffle your hair a bit, or someone might think you're with the Navy!",
		"If you have a weapon on you, make sure it remains out of sight. The crews of the Skalawag and the Black Death duked it out in here a couple of hours ago, and some people got killed. The last thing we need right now is someone waving his gun around.",
		"Some idiots from the ISO came in here yesterday handing out phamphlets and bothering the customers. They wouldn't take a hint, so I had them thrown out. I don't know who's worst, the ISO or those religious fanatics they fight with all the time.",
                "What are ye pesterin' me for, pal? If you want to be a pirate, you should go talk to the three important-looking pirates in the next room! Wait a second...",
                "Kasper Foch, now there's a guy you don't want to mess with. He's the pirate captain that entered an ion storm and flew out again without a scratch, while his three militia pursuers were never heard of again. Then there's this story going around of how he single-handedly took out a Confed Missile Cruiser, but if ya want my opinion, I think that one's bunk.",
                "There's a lot of deserters from the Navy around here these days. Purist and Indep punks they drafted off the street, taught how to fly and then handed a ship. You figure the guys at HQ would be smarter than that. At least we aren't seeing any Andolian deserters, if there even are such things. Most of us around here figure that if we ever see one, it's a sure sign the war is lost.",
                "A Rlaan transport can be a good target, if you're willing to take the risk. If those guys see you attacking one of their unarmed ships... well, sometimes they just won't stop hunting you! And you've heard what they do to humans they capture alive, haven't you?",
                "You know, if we lose the war against the Aera, it probably won't matter much, because the majority of us won't be around to see it, but what if we win? The whole fleet would return to peacekeeping duties in human systems, and it would mean the end of space piracy as we know it!",
                "Hey, I hope your ship has a large cargo hold. We could always use some more smugglers out there, distributing our goods. Can you believe that there are some colonists out there who still don't know the true glory of Khaisalantimin F62, or as most people just call it, 'khais'?",
                "This assassin came in here a while ago. Ex-Special Forces guy. Apparently, the government sent some of his collegues to silence him for good after he'd carried out a particularly nasty mission in Rlaan space, but he made it out and now he'll do any job for money.",),
	"refinery":("Sorry for the broken glass on the floor. The base was shook up a bit a couple of hours ago. Apparently, some guy on approach lost control and crashed right into the side of the station. Don't worry, it's nothing our damage control crews can't handle, but the poor guy who crashed won't be flying again. Or breathing, for that matter.",
	        ),

	"caribbean":("The buzz is that they're going to be shooting Dominique Moore's new holoVid on this planet soon! 'Greased Love Lightning' is the working title.",
	                     "Did you ever try that drug that was the Big Thing a few years ago? What was it... Brilliance? I liked it, but everyone said it was too popular on those dreary mining bases to be hip anymore. Oh well...",
	                     "We had a group of Confed Marines come through here last week on shore leave. Boy, did those guys know how to party! 'Train hard, play hard!' they said!",
	                     "Did you catch 'A Private Little Paradise' starring Miranda Podssun? It's the number one holoVid in the Confederation!",
	                     "Some people were spreading rumors about seeing a shark down by the beach. But that's ridiculous! There aren't any shark-analogues on this world, and who in their right mind would BRING a shark to a resort planet?",
	                     "Now that the Aera-Rlaan conflict is cooling off, surplus Rlaan medical supplies are making their way over to Terran space. Word is that there is some pretty potent stuff in those alien aid packs! The trick is choosing which injection will trip you, and which will just plain kill you.",
	                     "I dated a privateer for a while once. Then he got stupid and tried to run advanced Aera weapon supplies to Confed researchers. The pay from the first few missions was good, but too bad he died before he could spend any of it on me!",
	                     "I'd watch your step. That guy over there's had his eye on you since you came in. He's either some kind of shady character with a hidden agenda, or he has a crush on you!",
	                     "Don't blame the messenger, but that Shmrn over there just bought you a drink.",
	                     "I'm really glad I live on a pleasure resort like this one... I visited a farming colony once a few years ago, and smelled like manure and bad beer for a month!",
	                     "This colony just joined an advertising cooperative to promote tourism and interstellar vacations in the Confederation. I can't wait until business starts booming!",
	                     "One of the great things about living in a place like this is how there are hardly any Luddites or ISO commies! There's no large scale technology anywhere to bomb, and everyone here is either rich or plans on getting rich, and has no time for their malcontent whining. I love it!",
	                     "Welcome, friend! You look like you could use a quality libation from the lounge.",
	                     "There's going to be a big party tonight down the street. A bunch of Confed flyboys are going back to their ships after two weeks of shoreleave, and they want to have the biggest party the planet's ever seen!"
	                     "Here's to the Confederation Navy! On the front lines, fighting for our right to sex, drugs, and... to party!",
	                     "Feel that warm ocean breeze floating in through those big sun-filled windows? That's why I love working here.",
	                     "I've been lookin' for my lost shaker of salt. Have you seen it? Well, it's my own damn fault.",
	                     "Some people say that living on a pleasure planet isn't dangerous. But you won't believe what happened yesterday. Someone stepped on a Terran jellyfish on the beach, and I had to rush down there with my first aid kit and sprinkle the antidote on his foot!",
	                     "I'd love to chat, but I've got a bunch of customers coming in off the beach! It'll be a few minutes.",
	                     "Who do you think is the sexiest holoactress? Dominique Moore, Miranda Podssun, or Aika Nagomo?",
	                     "I actually don't plan on being a bartender for the rest of my life... My boyfriend right now is trying to get me into the modeling agency he works for! I can see it now... my brand new face and body splashed across holoscreens and starscrapers across the Confederation!",
	                     "I learned everything I know about bartending from a retired pilot named Shotglass. He... 'introduced' me to a lot of important concepts.",
	                     "There was a really good live band in here a few nights ago. Can you believe it! A live band! You usually only see those out on the fringe... or in museums. The customers seemed to enjoy it, but a lot of the younger people left to go to an autosynth show.",
	                     "Wouldn't you hate to grow up on a refinery station or a mining base? I mean, not only would there not be many people your own age... but you'd have to live with the grey, muted colors day in, and day out. Ugh.",
	                     "There was an unexpected downpour of rain a few hours ago, but the sun has broken back through the clouds and there's a rainbow! What are you still doing inside, go out and look at it!",
	                     "You should see some of the rubes who come through here on vacation. They come from a bland suburb on some standard marginal planet, stay here a week, and go back vibrant and full of life! I've heard that those people are thirty-seven times more likely to snap and massacre their coworkers within six months of their return, though. I wonder what that means.",
	                     "It seems like us pleasure planets are doing really well, what with the war and all. People just want to be able to get away from it all, and forget that there is an empire full of cruel aliens bent on our annihilation. Here, next one's on me.",
	                     "You picked a fine time to come to here, sir. With all the bad news about lately, everyone is anxious to find a little getaway. Weve got plenty of various entertainments for everyone. I think youll find your stay here the best youve ever enjoyed.",),

	"iso":("4B4ND0N TH3 R4NK5 0F TH3 18M 80URG301513! L1NU5 M4K35 C0MPUT3R5 F0R TH3 M0DERN PR0L3T4R14T3!",
			                    "J00 H4V3 N0TH1NG T0 L053 8UT J00R CH41N5!",
			                    "FR0M 34CH 4CC0RD1NG T0 TH31R M34N5, T0 34CH 4CC0RD1NG T0 TH31R N33D5",
			                    "4LL P0W3R T0 TH3 P30P13",
			                    "GR33D H4S P01S0N3D M3N'5 50UL5 -- H4S B4RR1C4D3D TH3 W0RLD W1TH H4T3 -- H45 G0053ST3PP3D US 1NT0 M1S3RY 4ND BL00D5H3D.",
			                    "W3 H4V3 D3V3L0P3D 5P33D, BUT W3 H4V3 5HUT OUR53LV35 IN M4CH1N3RY TH4T G1V35 4BUND4NC3 H45 L3FT U5 1N W4NT. 0UR KN0WL3DG3 H45 M4D3 U5 CYNIC4L; OUR CL3V3RN355 H4RD 4ND UNKIND.",
			                    "W3 THINK TOO MUCH 4ND F33L TOO LITTL3.",
			                    "MOR3 TH4N M4CHIN3RY W3 N33D HUM4NITY.",
			                    "MOR3 TH4N CL3V3RN355 W3 N33D KINDN355. WITHOUT TH353 QU4LITI35 LIF3 WILL B3 VIOL3NT 4ND 4LL WILL B3 LO55",
			                    "W3 4R3 COMING OUT OF TH3 D4RKN355 INTO TH3 LIGHT!",
			                    "W3 4R3 COMING INTO 4 N3W WORLD! LOOK UP H4NN4H! TH3 5OUL OF M4N H45 B33N GIV3N WING5 4ND 4T L45T H3 I5 B3GINNING TO FLY.",
			                    )
			             }

speaktimes=0
def GetDefaultBartenderText():
    txt =tender.get("general")
    if (not txt):
        return ("Hello",)
    return txt

def GetBartenderText(str):
    txt = tender.get (str)
    if (txt):
        return txt
    return GetDefaultBartenderText()

def Speak(thingstosay):
    try:
      Base.EraseObj(Base.GetCurRoom(),"BartenderText")
    except:
      pass
    (text,sound)=Base.GetRandomBarMessage()
    rndnum=vsrandom.randrange(0,2)
    roomNumber=Base.GetCurRoom()
    textBox = "BartenderText"
    xCoord=-0.5				#float, -1.0 to 1.0, test range -0.8 to -0.3
    yCoord=0.85				#float, 1.0 to -1.0, test range 0.5 to 0.9
    WidHeiMult=(0.5, -0.7, 1.0) 	#Tuple of values, (float Width, float Height, float Multiplier)
    		# Width range 0.0 to 1.0, Height range -1.0 to 0.1, Multiplier range 0.0 to ?
		# test range 0.3 to 0.7, Height 0.1 is fine TextBox will expand to fit text, Multiplier does not seem to work
    BGColor=(.0, .0, .0)		# textbox background color Vector? Are those RGB values?
    BackAlp=0.0				# Background alpha transparancy.
    		# I recommend leaving BGColor to (0.0, 0.0, 0.0) and BackAlp to 0.0
		# Note: without a transparant background, g's p' y's and such get clipped.
    FGColor=(1.0, 1.0, 1.0)		# RGB values (0.0, 0.0, 0.0) to (1.0, 1.0, 1.0) low to high or black to white

    # This section will create the ok button, which in turn will destroy the TextBox when the button is pushed
    fixers.DestroyActiveButtons()
    fixers.CreateChoiceButtons(roomNumber, [fixers.Choice("/bases/fixers/yes.spr","bases/bar-ok.py","Thanks For The Talk, Doc")])

    if (rndnum==1 or text==''):
        mylen=len(thingstosay)
	if (mylen>0):
            # replace this line
            #Base.Message (thingstosay[vsrandom.randrange(0,mylen)])
	    Base.TextBox(roomNumber, textBox, thingstosay[vsrandom.randrange(0,mylen)], xCoord, yCoord, WidHeiMult, BGColor, BackAlp, FGColor)
        else:
            # replace this line
            #Base.Message ('Hello!')
	    Base.TextBox(roomNumber, textBox, 'Hello!', xCoord, yCoord, WidHeiMult, BGColor, BackAlp, FGColor)
    else:
        # replace this line
        #Base.Message (text)
	Base.TextBox(roomNumber, textBox, text, xCoord, yCoord, WidHeiMult, BGColor, BackAlp, FGColor)
        if (sound!=''):
            VS.playSound (sound)




















