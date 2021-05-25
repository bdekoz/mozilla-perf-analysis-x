
# WHERE "time" > '2021-04-*/'
# WHERE "product" = 'fenix-nightly' AND "device" = 'pixel-2'
# FROM "main-post-onboard"
# FROM "view"

#influx -host hilldale-b40313e5.influxcloud.net -port 8086 -username performance_ro -password 8cd4376a09477b73aab63dadd3d893f1 -ssl -execute 'SELECT * FROM "view" WHERE "product" = "fenix-nightly" AND "device" = "pixel-2" LIMIT 1000' -database="performance" -precision=rfc3339

#influx -host hilldale-b40313e5.influxcloud.net -port 8086 -username performance_ro -password 8cd4376a09477b73aab63dadd3d893f1 -ssl -execute 'SELECT * FROM "view" WHERE ("device" =~ /pixel-2/) AND ("product" =~ /fenix*/)' -database="performance" -precision=rfc3339

# FirstVisualChange
# LastVisualChange
# firstPaint
influx -host hilldale-b40313e5.influxcloud.net -port 8086 -username performance_ro -password 8cd4376a09477b73aab63dadd3d893f1 -ssl -execute 'SELECT * FROM "firstPaint" WHERE ("device" =~ /pixel_4_xl/) AND ("product" =~ /fenix*/)' -database="performance" -precision=rfc3339

