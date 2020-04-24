Notes on the Organisation of Websites for Ginger
================================================

* The git repo can be at https://github/spicery/ginger
* The top-level README.md of the git repo is a natural landing page for developers
    - However it probably should provide links off to the implementation reference.

* Implementation reference should be under: http://dev.ginger.spicery.org/${APPNAME}
    - Overview would be under /overview
    - Each application's docs would be hosted in its own folder.
    - The site would be hosted via netlify.com
* Also standard library reference should be under http://ref.ginger.spicery.org/stdlib
    - Which would be hosted via netlify.com too. 
    - Ideally on the same website.
    - The sources would be held in the ginger-repo.

* The language reference guide should be in http://ref.ginger.spicery.org/lang
    - I am unclear where it should be mastered (ginger-repo or gingerdocs-repo?)

* Gingerdocs should host the programmers guide i.e. be in the gingerdocs-repo
    - It should have the URL http://docs.ginger.spicery.org/
    - It ought to include the cheatsheet.
