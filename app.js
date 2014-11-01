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
var Emotion = require('./models/emotion')(mongoose);

// Routes
// Main page
app.get('/', function(req, res) {
	res.send('hello');
});

// Add new emotion data
app.get('/at/:location/feeling/:pulse/', function(req, res) {
	var emotion = new Emotion({
		nmea:		req.params.location,
		heartbeat:	req.params.pulse,
		when:		new Date
	});
	emotion.save(function(err, emotion) {
		if (err) return console.error(err);
		res.setHeader('Content-Type', 'application/json');
		res.end(JSON.stringify(emotion));
	});
});

// Get all emotion data
app.get('/data/', function(req, res) {
	Emotion.find(function(err, emotions) {
		if (err) return console.error(err);
		res.setHeader('Content-Type', 'application/json');
		res.end(JSON.stringify(emotions));
	});
});

// Remove all emotion data; for test purpose
app.get('/brainwash/', function(req, res) {
	Emotion.remove({}, function() {});
	Emotion.find(function(err, emotions) {
		if (err) return console.error(err);
		res.setHeader('Content-Type', 'application/json');
		res.end(JSON.stringify(emotions));
	});
});

// Remove the last emotion data; for test purpose
app.get('/forget/about/it/', function(req, res) {
	Emotion.findOne({}, {}, { sort: { 'when': -1 } }, function(err, emotion) {
		if (err) return console.error(err);
		emotion.remove();
		res.setHeader('Content-Type', 'application/json');
		res.end(JSON.stringify(emotion))
	});
});

// Listen!
var port = process.env.PORT || 4444;
server.listen(port, function() {
	console.log('Server listening for HTTP connection on port %d', port);
});