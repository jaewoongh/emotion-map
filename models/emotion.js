module.exports = function(mongoose) {
	var EmotionSchema = new mongoose.Schema({
		nmea:		String,
		heartbeat:	Number,
		when:		Date
	});
	return mongoose.model('Emotion', EmotionSchema);
};