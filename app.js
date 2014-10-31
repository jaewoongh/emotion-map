// Set up express and server
var express = require('express');
var app = express();
var server = require('http').Server(app);

// Set up MongoDB with Mongoose
var mongoose = require('mongoose');
mongoose.connect('mongodb://user:shinymetalass@ds049160.mongolab.com:49160/emotion-map');
var db = mongoose.connection;
db.on('error', console.error.bind(console, 'MongoDB connection error: '));
db.once('open', function() { console.log('MongoDB successfully connected'); });

// Listen!
var port = process.env.PORT || 4444;
server.listen(port, function() {
	console.log('Server listening for HTTP connection on port %d', port);
});