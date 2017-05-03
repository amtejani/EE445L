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

State = "0"
DoorStatus = "0"
LockStatus = "1"
Keycode = "0000"

class MainPage(webapp2.RequestHandler):
    def get(self):
        self.response.write('<html><body>')
        self.response.write('<h2>Security System. By Ali and Caroline</h2>')
        self.response.write('<hr>')
		
        s = ""
        if (State == '0'):
		    s = "Off"
        elif (State == '1'):
			s = "Enabled"
        elif (State == '2'):
			s = "Door breached"
        elif (State == '3'):
			s = "Code being changed (1)"
        elif (State == '4'):
			s = "Code being changed (2)"
        elif (State == '5'):
			s = "Code being changed (3)"
        else: 
            s = "Alarm will turn on in 5s"
			
        d = ""
        if (DoorStatus == "0"):
			d = "Open"
        else: 
            d = "closed"
			
        l = ""
        if (LockStatus == "1"):
			l = "Locked"
        else: 
            l = "Unlocked"
	
        self.response.write('<br>State: %s<br>Door: %s<br>Lock: %s' %
									(s, d, l))
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
    def get(self):
        global Keycode
        old = self.request.get('oldcode')
        new1 = self.request.get('newcode1')
        new2 = self.request.get('newcode2')
        if (old == Keycode and new1 == new2):
            Keycode = new1
            self.response.write('succ=1')
        self.response.write('succ=0')
		
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
    def get(self):
        #comment these three lines for debugging; So we can access it through a browser
        code = self.request.get('code')
        if code == Keycode:
            self.response.write('good=1')
        self.response.write('good=0')
		
		
class Status(webapp2.RequestHandler):
    def get(self):
        #comment these three lines for debugging; So we can access it through a browser
        global State
        global DoorStatus
        state = self.request.get('state')
        door = self.request.get('door')
        State = state
        DoorStatus = door
        self.response.write('lock=' + LockStatus)
		


application = webapp2.WSGIApplication([
  ('/', MainPage),
  ('/changecode', ChangeCode),
  ('/changelock', ChangeLock),
  ('/changestate', ChangeState),
  ('/checkcode', CheckCode),
  ('/status', Status)
], debug=True)
