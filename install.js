exports.install = function() {
  require('juice/installer').install(module.id)
}

if (require.main === module)
  exports.install();
