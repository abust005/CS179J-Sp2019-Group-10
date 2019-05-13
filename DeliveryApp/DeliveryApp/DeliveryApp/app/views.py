"""
Definition of views.
"""

from django.shortcuts import render
from django.http import HttpRequest
from django.template import RequestContext
from datetime import datetime
from django.shortcuts import redirect
from app.models import Destination
from django.contrib.gis.geos import Point

def home(request):
    """Renders the home page."""
    #Destination(id=1, title="Source", geom=Point(30,30)).save()
    return render(
        request,
        'app/home.html',
        {
        }
    )

def confirm(request):
    """Renders the confirmation page."""
    packageID = ""
    destination = ""

    if request.method == 'POST':
        if request.POST.get('packageID'):
            packageID = request.POST['packageID']
        if request.POST.get('destination'):
            destination = request.POST['destination']
        if request.POST.get('isValid'):
            print("Yes")
            return redirect('home')
        elif request.POST.get('isNotValid'):
            print("No")
            return redirect('home')
    return render(
        request,
        'app/confirm.html',
        {
            'packageID': packageID,
            'destination': destination
        }
    )

def about(request):
    """Renders the about page."""
    return render(
        request,
        'app/about.html',
        {
        }
    )
