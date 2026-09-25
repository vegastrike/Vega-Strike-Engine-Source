"""Scenarios: which branches the automated player takes and how the mock
universe is tuned (hit points, damage...)."""

# Diverting Kaydence's escort to Newcastle (the ISO route) is recorded by
# wrong_escort as awacs_escort=-1 (its initial value) and the escort
# mission itself as failed (-1): both are the intended branch mechanics.
ISO_BRANCH_FAILURES = {('wrong_escort', 'awacs_escort'): 1,
                       ('wrong_escort', 'freetrader_campaign_mission'): 1}

MAIN = 'privateer_campaign'
RF = 'rf_campaign'
RF_MURPHY = 'rf_murphy_campaign'
RF_GOODIN = 'rf_goodin_campaign'
RF_TAYLA = 'rf_tayla_campaign'
RF_LYNCH = 'rf_lynch_campaign'
BONUS = 'freetrader_campaign'


class Phase:
    def __init__(self, name, campaigns, required, optional_visits=None):
        self.name = name
        self.campaigns = campaigns          # campaigns worked on in this phase
        self.required = required            # phase ends when these reach their end node
        self.optional_visits = optional_visits or {}   # campaign -> max fixer visits


DEFAULT_PHASES = [
    Phase('Privateer', [MAIN], [MAIN]),
    # rf_campaign itself never reaches an end node (it parks on the "library
    # is closed" click node), the finale is in the Goodin/Terrell line.
    Phase('Righteous Fire', [RF, RF_MURPHY, RF_TAYLA, RF_GOODIN, RF_LYNCH], [RF_GOODIN],
          optional_visits={RF_LYNCH: 2}),
    Phase('Bonus (ISO)', [BONUS], [BONUS]),
]


class Scenario:
    """Knobs for one automated playthrough.

    choices: maps a choice button text (e.g. 'Accept_Undercover') or a
      policy key (e.g. 'dump_contraband', 'divert_escort') to a value.
    refuse_first: refuse every mission offer once before accepting, which
      walks the reject/reconsider dialog branches too.
    fail_missions: set of campaign save variables whose mission should be
      made to fail (by making the protected NPCs fragile) - see run_campaign.
    """

    def __init__(self, name='full', phases=None, choices=None, refuse_first=False,
                 unlock_bonus=True, engine_options=None, stop_after_phase=None,
                 max_game_seconds=5.0e6, fragile=(), doomed=()):
        self.name = name
        self.phases = phases or DEFAULT_PHASES
        self.choices = dict(choices or {})
        self.refuse_first = refuse_first
        self.unlock_bonus = unlock_bonus
        self.engine_options = dict(engine_options or {})
        self.stop_after_phase = stop_after_phase
        self.max_game_seconds = max_game_seconds
        # [(mission type, flightgroup prefix)]: ships launched by that
        # mission in that flightgroup get 1 hit point (to force failures)
        self.fragile = list(fragile)
        # [(mission type, flightgroup prefix, seconds)]: such ships are
        # destroyed that long after launch (deterministic failures)
        self.doomed = list(doomed)

    def hp_overrides(self):
        out = []
        for mtype, fgprefix in self.fragile:
            out.append((lambda u, mt=mtype, fp=fgprefix:
                        u.launched_by == mt and u.fg_name().startswith(fp), 1.0))
        return out

    def choice(self, key, default=None):
        return self.choices.get(key, default)


SCENARIOS = {
    # Accept everything, take the ISO (Demetria) route through the bonus
    # campaign by diverting Kaydence's escort through Newcastle.
    'full': Scenario('full', choices={'divert_escort': True, 'dump_contraband': True,
                                      'allowed_failures': ISO_BRANCH_FAILURES}),
    # Same but also refuse every offer once first (exercises reject paths).
    'full-refuse-first': Scenario('full-refuse-first', choices={'divert_escort': True, 'dump_contraband': True,
                                                                'allowed_failures': ISO_BRANCH_FAILURES},
                                  refuse_first=True),
    # Bonus campaign only: go undercover for Destinee but deliver Syrai's
    # contraband anyway (no dump), which leads through Syrai to Soren.
    'bonus-syrai-soren': Scenario('bonus-syrai-soren', choices={'divert_escort': False},
                                  phases=[DEFAULT_PHASES[2]]),
    # Only the main campaign.
    'main': Scenario('main', phases=[DEFAULT_PHASES[0]]),
    # Bad outcomes: Hunter Toth (Masterson's first escort) gets killed; the
    # main campaign must end cleanly on its failure branch.
    'main-escort-dies': Scenario('main-escort-dies', phases=[DEFAULT_PHASES[0]],
                                 fragile=[('escort_local', 'Escort')],
                                 choices={'allowed_failures': {('escort_local', 'privateer_campaign_mission'): 1}}),
    # The senator (Kaydence 3, escort_mission) dies: the bonus campaign's
    # no-failure mission then branches to the ISO (Demetria) arc.
    'bonus-senator-dies': Scenario('bonus-senator-dies', phases=[DEFAULT_PHASES[2]],
                                   choices={'dump_contraband': True,
                                            'allowed_failures': {('escort_mission', 'freetrader_campaign_mission'): 1}},
                                   doomed=[('escort_mission', 'Escort', 3.0)]),
}
