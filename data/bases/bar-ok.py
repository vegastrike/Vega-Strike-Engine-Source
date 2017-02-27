import fixers
import Base
# Don't like relying on a string value that was not passed, but for now, that is the only way
Base.EraseObj(Base.GetCurRoom(),"BartenderText")
fixers.DestroyActiveButtons ()

