#### Script for EdX course UT6.02X
## Author: Ramesh Yerraballi
## Date: Spring 2015
## License: Freeware
## The code here is based on three sources:
##  1. The basic guestbook appengine application from Google
##  2. pymaps.py script from: https://code.google.com/p/pymaps/
##  3. ipinfo.io's ip locator is more accurate than geobytes http://ipinfo.io/
##  4. geobytes.com's ip locator at http://www.geobytes.com/iplocator/
##     and their json api
## Need the next line to deal with strings that the geobytes server returns that
## have unicode characters -- Did not Work though
#-*- coding: UTF-8 -*-
import cgi
import time
import logging
import urllib2
import unicodedata
import webapp2
import random
import json
from google.appengine.ext import ndb

State = "Off"
DoorStatus = ""
LockStatus = ""
Keycode = "0000"

class MainPage(webapp2.RequestHandler):
    def get(self):
        self.response.write('<html><body>')
        self.response.write('<h2>Security System. By Ali and Caroline</h2>')
        self.response.write('<hr>')
	
        self.response.write('<br>State: %s<br>Door: %s<br>Lock: %s' %
									(State, DoorStatus, LockStatus))
        self.response.write('<hr>')
        self.response.write("""<form action="/changecode" method="post">
            <div><input type="text" name="oldcode"></div>
            <div><input type="text" name="newcode1"></div>
            <div><input type="text" name="newcode2"></div>
            <div><input type="submit" value="Change Code"></div>
          </form>
		  <form action="/changelock" method="post">
            <div><input type="text" name="code"></div>
            <div><input type="radio" name="lock" value="unlock">Unlock</div>
            <div><input type="radio" name="lock" value="lock">Lock</div>
            <div><input type="submit" value="Lock/Unlock"></div>
          </form>
		  <form action="/changestate" method="post">
            <div><input type="text" name="code"></div>
            <div><input type="radio" name="state" value="off">Off</div>
            <div><input type="radio" name="state" value="on">On</div>
            <div><input type="submit" value="Change State"></div>
          </form>
          <hr>
        </body>
      </html>""")

class ChangeCode(webapp2.RequestHandler):
    def post(self):
        global Keycode
        old = self.request.get('oldcode')
        new1 = self.request.get('newcode1')
        new2 = self.request.get('newcode2')
        if (old == Keycode and new1 == new2):
            Keycode = new1
        self.redirect('/')
		
class ChangeLock(webapp2.RequestHandler):
    def post(self):
        global LockStatus
        code = self.request.get('code')
        lock = self.request.get('lock')
        if (code == Keycode):
            LockStatus = '1' if (lock == 'lock') else '0'
        self.redirect('/')
		
class ChangeState(webapp2.RequestHandler):
    def post(self):
        global State
        code = self.request.get('code')
        state = self.request.get('state')
        if (code == Keycode):
            State = '1' if (state == 'on') else '0'
        self.redirect('/')
		
class CheckCode(webapp2.RequestHandler):
    def post(self):
        #comment these three lines for debugging; So we can access it through a browser
        if self.request.environ.get('HTTP_USER_AGENT') != 'Keil':
           self.response.write('<html><body> <pre> Invalid Access</pre> </body></html>')
           return
        code = self.request.get('code')
        if code == Keycode:
            self.response.write('true')
        self.response.write('false')
		
		
class Status(webapp2.RequestHandler):
    def post(self):
        #comment these three lines for debugging; So we can access it through a browser
        if self.request.environ.get('HTTP_USER_AGENT') != 'Keil':
           self.response.write('<html><body> <pre> Invalid Access</pre> </body></html>')
           return
        global State
        global DoorStatus
        state = self.request.get('state')
        door = self.request.get('door')
        State = state
        DoorStatus = door
		


application = webapp2.WSGIApplication([
  ('/', MainPage),
  ('/changecode', ChangeCode),
  ('/changelock', ChangeLock),
  ('/changestate', ChangeState),
  ('/checkcode', CheckCode),
  ('/status', Status)
], debug=True)
