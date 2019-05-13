"""
Definition of urls for DeliveryApp.
"""

from datetime import datetime
from django.conf.urls import url
import django.contrib.auth.views
from django.conf import settings
from django.conf.urls.static import static
from djgeojson.views import GeoJSONLayerView
from app.models import Destination
from django.contrib import admin

import app.forms
import app.views

admin.site.register(Destination)

urlpatterns = [
    url(r'^admin/', admin.site.urls), # admin site url
    url(r'^$', app.views.home, name='home'),
    url(r'^confirm$', app.views.confirm, name='confirm'),
    url(r'^about$', app.views.about, name='about'),
    url(r'^data/$', GeoJSONLayerView.as_view(model=Destination), name='data')
]
